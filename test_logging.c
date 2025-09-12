#include "include/demi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Test logging function
    printf("Testing demi_log function...\n");
    
    // Test basic logging
    demi_log("Test message: logging system is working");
    demi_log("Another test message with special chars: !@#$%^&*()");
    
    // Test with device events simulation
    demi_log("netlink event: device=sda1 action=add");
    demi_log("device filter: device=sda1 allowed=yes");
    demi_log("netlink event: device=usb0 action=remove");
    demi_log("device filter: device=usb0 allowed=no");
    
    printf("Logging test completed. Check /var/log/devd-watcher.log\n");
    return 0;
}
