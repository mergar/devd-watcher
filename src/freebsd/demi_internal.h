#include <sys/param.h>
#include <sys/socket.h>

#define DEMI_CLOEXEC SOCK_CLOEXEC
#define DEMI_NONBLOCK SOCK_NONBLOCK

//usr/include/sys/param.h:#define SPECNAMELEN    255             /* max length of devicename */
//#define DEMI_DEVNAME_MAX (SPECNAMELEN + 1)

#define DEMI_MONITOR_DEVD_SOCKET "/var/run/devd.seqpacket.pipe"
