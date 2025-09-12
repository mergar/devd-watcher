#ifndef _DEMI_H_
#define _DEMI_H_

//usr/include/sys/param.h:#define SPECNAMELEN    255             /* max length of devicename */

/* Public limit for device name length */
#ifndef DEMI_DEVNAME_MAX
#define DEMI_DEVNAME_MAX (255 + 1)
#endif

enum demi_event_type {
    DEMI_UNKNOWN,
    DEMI_ATTACH,
    DEMI_DETACH,
    DEMI_CHANGE,
    // DEMI_HOTPLUG,
    // DEMI_LEASE,
};

struct demi_event {
    char de_devname[DEMI_DEVNAME_MAX];
    enum demi_event_type de_type;
};

int demi_init(int flags);
int demi_read(int fd, struct demi_event *event);

#endif
