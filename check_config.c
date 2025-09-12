#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct config {
    char *lock_dir;
    int lock_timeout_seconds;
    char *allowed_devices;
};

static struct config g_config = {
    .lock_dir = NULL,
    .lock_timeout_seconds = 5,
    .allowed_devices = NULL
};

static void trim_whitespace(char *str) {
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    char *start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static int parse_config_file(const char *config_path) {
    printf("Opening config file: %s\n", config_path);
    FILE *file = fopen(config_path, "r");
    if (!file) {
        printf("ERROR: Cannot open file: %s\n", strerror(errno));
        return -1;
    }

    char line[512];
    int line_num = 0;
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        printf("Line %d: '%s'", line_num, line);
        
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            printf("  -> Skipped (comment/empty)\n");
            continue;
        }

        // Find the '=' character
        char *equals = strchr(line, '=');
        if (!equals) {
            printf("  -> Skipped (no '=' found)\n");
            continue;
        }

        // Split the line into key and value
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        // Trim whitespace
        trim_whitespace(key);
        trim_whitespace(value);

        printf("  -> Key: '%s', Value: '%s'", key, value);

        // Remove quotes if present
        if (value[0] == '"' && value[strlen(value) - 1] == '"') {
            value[strlen(value) - 1] = '\0';
            value++;
            printf(" (quotes removed: '%s')", value);
        }

        // Parse configuration parameters
        if (strcmp(key, "DEMI_LOCK_DIR") == 0) {
            free(g_config.lock_dir);
            g_config.lock_dir = strdup(value);
            printf("  -> Set DEMI_LOCK_DIR to: '%s'\n", g_config.lock_dir);
        } else if (strcmp(key, "DEMI_LOCK_TIMEOUT_SECONDS") == 0) {
            g_config.lock_timeout_seconds = atoi(value);
            if (g_config.lock_timeout_seconds <= 0) {
                g_config.lock_timeout_seconds = 5;
            }
            printf("  -> Set DEMI_LOCK_TIMEOUT_SECONDS to: %d\n", g_config.lock_timeout_seconds);
        } else if (strcmp(key, "DEMI_ALLOWED_DEVICES") == 0) {
            free(g_config.allowed_devices);
            g_config.allowed_devices = strdup(value);
            printf("  -> Set DEMI_ALLOWED_DEVICES to: '%s'\n", g_config.allowed_devices);
        } else {
            printf("  -> Unknown parameter: '%s'\n", key);
        }
    }

    fclose(file);
    return 0;
}

static void cleanup_config(void) {
    free(g_config.lock_dir);
    free(g_config.allowed_devices);
}

int main(int argc, char *argv[])
{
    const char *config_file = "etc/devd-watcher.conf";
    
    if (argc > 1) {
        config_file = argv[1];
    }

    printf("=== Config File Parser Test ===\n");
    printf("Testing file: %s\n\n", config_file);

    // Parse configuration file
    if (parse_config_file(config_file) == -1) {
        printf("\nERROR: Failed to parse config file\n");
        return EXIT_FAILURE;
    }

    // Register cleanup function
    atexit(cleanup_config);

    // Print final configuration
    printf("\n=== Final Configuration ===\n");
    printf("DEMI_LOCK_DIR: %s\n", g_config.lock_dir ? g_config.lock_dir : "(not set)");
    printf("DEMI_LOCK_TIMEOUT_SECONDS: %d\n", g_config.lock_timeout_seconds);
    printf("DEMI_ALLOWED_DEVICES: %s\n", g_config.allowed_devices ? g_config.allowed_devices : "(not set)");

    printf("\n=== Test completed successfully ===\n");
    return EXIT_SUCCESS;
}
