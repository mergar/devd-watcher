#ifndef _DEMI_INTERNAL_LINUX_H_
#define _DEMI_INTERNAL_LINUX_H_

#include <sys/socket.h>

#define DEMI_CLOEXEC SOCK_CLOEXEC
#define DEMI_NONBLOCK SOCK_NONBLOCK

#define DEMI_DEVNAME_MAX (255 + 1)

/* Netlink multicast group for uevents */
#define DEMI_MONITOR_NETLINK_GROUP 0x1

#endif /* _DEMI_INTERNAL_LINUX_H_ */
