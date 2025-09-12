#include "include/demi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("Testing demi logging with device filtering...\n");
    
    // Set allowed devices (same as in config)
    demi_set_allowed_devices("cd* vtbd* ada* md[0-3]");
    
    // Test device filtering and logging
    const char *test_devices[] = {
        "sda1",    // Should be blocked
        "cd0",     // Should be allowed
        "vtbd0",   // Should be allowed  
        "ada0",    // Should be allowed
        "md0",     // Should be allowed
        "md1",     // Should be allowed
        "md2",     // Should be allowed
        "md3",     // Should be allowed
        "md4",     // Should be blocked
        "usb0",    // Should be blocked
        NULL
    };
    
    for (int i = 0; test_devices[i] != NULL; i++) {
        const char *device = test_devices[i];
        int allowed = demi_is_device_allowed(device);
        
        // Simulate logging as it would happen in demi_read
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "netlink event: device=%s action=add", device);
        demi_log(log_msg);
        
        snprintf(log_msg, sizeof(log_msg), "device filter: device=%s allowed=%s", 
                 device, allowed ? "yes" : "no");
        demi_log(log_msg);
        
        printf("Device %s: %s\n", device, allowed ? "ALLOWED" : "BLOCKED");
    }
    
    printf("Test completed. Check /var/log/devd-watcher.log\n");
    return 0;
}
