#include "include/demi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/file.h>

#ifndef DEMI_PLATFORM_FREEBSD
#ifdef __FreeBSD__
#define DEMI_PLATFORM_FREEBSD 1
#endif
#endif

#ifndef DEMI_LOCK_TIMEOUT_SECONDS
#define DEMI_LOCK_TIMEOUT_SECONDS 5
#endif

struct helper_args {
    char *command;
    char dev_basename[256];
};

/* Select lock directory per-platform */
#if defined(DEMI_PLATFORM_FREEBSD) || defined(MI_PLATFORM_FREEBSD)
#define DEMI_LOCK_DIR "/var/run/devd-watcher"
#else
#define DEMI_LOCK_DIR "run"
#endif

static int ensure_lock_directory(const char *dir_path)
{
    struct stat st;
    if (stat(dir_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        return -1;
    }
    if (errno != ENOENT) {
        return -1;
    }
    return mkdir(dir_path, 0755);
}

static int acquire_lock_with_timeout(const char *lock_path, int timeout_seconds, int *out_fd)
{
    int open_flags = O_CREAT | O_RDWR;
#ifdef O_CLOEXEC
    open_flags |= O_CLOEXEC;
#endif
    int fd = open(lock_path, open_flags, 0644);
    if (fd == -1) {
        return -1;
    }

    struct timespec sleep_req = {0};
    sleep_req.tv_sec = 0;
    sleep_req.tv_nsec = 100000000L; /* 100ms */

    time_t start = time(NULL);
    for (;;) {
        if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
            /* On successful lock, record timestamp and owner into the lock file */
            (void)lseek(fd, 0, SEEK_SET);
            (void)ftruncate(fd, 0);
            time_t now = time(NULL);
            char lock_note[64];
            int note_len = snprintf(lock_note, sizeof(lock_note), "%ld:devd-watcher\n", (long)now);
            if (note_len > 0) {
                ssize_t ignored = write(fd, lock_note, (size_t)note_len);
                (void)ignored;
            }
            if (out_fd) {
                *out_fd = fd;
            }
            return 0;
        }
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            close(fd);
            return -1;
        }
        if (timeout_seconds >= 0 && (int)(time(NULL) - start) >= timeout_seconds) {
            close(fd);
            errno = EWOULDBLOCK;
            return -1;
        }
        nanosleep(&sleep_req, NULL);
    }
}

static void release_lock(int fd)
{
    (void)flock(fd, LOCK_UN);
    close(fd);
}

static void *run_helper(void *arg)
{
    struct helper_args *ha = (struct helper_args *)arg;
    if (ensure_lock_directory(DEMI_LOCK_DIR) == -1) {
        fprintf(stderr, "failed to ensure lock directory '%s': %s\n", DEMI_LOCK_DIR, strerror(errno));
        free(ha->command);
        free(ha);
        return NULL;
    }

    char lock_path[512];
    snprintf(lock_path, sizeof(lock_path), "%s/%s.lock", DEMI_LOCK_DIR, ha->dev_basename);

    int lock_fd = -1;
    if (acquire_lock_with_timeout(lock_path, DEMI_LOCK_TIMEOUT_SECONDS, &lock_fd) == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            fprintf(stderr, "lock busy for %s after %d seconds (path: %s), skipping\n", ha->dev_basename, (int)DEMI_LOCK_TIMEOUT_SECONDS, lock_path);
        } else {
            fprintf(stderr, "failed to acquire lock for %s (path: %s): %s\n", ha->dev_basename, lock_path, strerror(errno));
        }
        free(ha->command);
        free(ha);
        return NULL;
    }

    int rc = system(ha->command);
    (void)rc;

    release_lock(lock_fd);
    if (unlink(lock_path) == -1) {
        if (errno != ENOENT) {
            fprintf(stderr, "failed to remove lock file '%s': %s\n", lock_path, strerror(errno));
        }
    }
    free(ha->command);
    free(ha);
    return NULL;
}

int main(void)
{
    // Initialize demi file descriptor with no/zero flags.
    // Optionally, DEMI_CLOEXEC and DEMI_NONBLOCK can be bitwise ORed in flags
    // to atomically set close-on-exec flag and nonblocking mode respectively.
    int fd = demi_init(0);

    if (fd == -1) {
        return EXIT_FAILURE;
    }

    struct demi_event de;

#if defined(DEMI_PLATFORM_LINUX) || defined(MI_PLATFORM_LINUX)
#define DEMI_PLATFORM_NAME "linux"
#elif defined(DEMI_PLATFORM_FREEBSD) || defined(MI_PLATFORM_FREEBSD)
#define DEMI_PLATFORM_NAME "freebsd"
#else
#define DEMI_PLATFORM_NAME "unknown"
#endif

    

    // Read pending event. This call will block since DEMI_NONBLOCK was not set.
    while (demi_read(fd, &de) != -1) {
        // de_devname might not contain devname, indicating that the event shall be ignored.
        if (de.de_devname[0] == '\0') {
            continue;
        }

        // Prepend /dev/ to devname, so that we have full path to devnode.
        char devnode[sizeof(de.de_devname) + sizeof("/dev/")];
        snprintf(devnode, sizeof(devnode), "/dev/%s", de.de_devname);

        const char *action = NULL;
        switch (de.de_type) {
            case DEMI_ATTACH: action = "attach"; break;
            case DEMI_DETACH: action = "detach"; break;
            case DEMI_CHANGE: action = "change"; break;
            default: break;
        }

        if (action) {
            size_t needed = (size_t)snprintf(NULL, 0, "helpers/%s/%s %s", DEMI_PLATFORM_NAME, action, devnode) + 1;
            struct helper_args *ha = (struct helper_args *)malloc(sizeof(*ha));
            if (!ha) {
                continue;
            }
            ha->command = (char *)malloc(needed);
            if (!ha->command) {
                free(ha);
                continue;
            }
            (void)snprintf(ha->command, needed, "helpers/%s/%s %s", DEMI_PLATFORM_NAME, action, devnode);

            /* derive basename from devnode path */
            const char *slash = strrchr(devnode, '/');
            const char *base = slash ? slash + 1 : devnode;
            size_t base_len = strlen(base);
            if (base_len >= sizeof(ha->dev_basename)) {
                /* Truncate to fit and warn */
                memcpy(ha->dev_basename, base, sizeof(ha->dev_basename) - 1);
                ha->dev_basename[sizeof(ha->dev_basename) - 1] = '\0';
                fprintf(stderr, "device basename too long, truncated: %s\n", ha->dev_basename);
            } else {
                memcpy(ha->dev_basename, base, base_len + 1);
            }

            pthread_t tid;
            if (pthread_create(&tid, NULL, run_helper, ha) == 0) {
                pthread_detach(tid);
            } else {
                free(ha->command);
                free(ha);
            }
        }
    }

    // Do not forget to close file descriptor when you are done.
    close(fd);
    return EXIT_SUCCESS;
}
