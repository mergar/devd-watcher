#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

// Copy the demi_log function for testing
void demi_log(const char *message) {
    const char *log_file = "/var/log/devd-watcher.log";
    
    FILE *log_fp = fopen(log_file, "a");
    if (!log_fp) {
        printf("Cannot open log file: %s\n", log_file);
        return;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    // Format time in syslog format: Sep 12 21:10:36
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M:%S", tm_info);
    
    // Write log entry in syslog format: Sep 12 21:10:36 devd-watcher: message
    fprintf(log_fp, "%s devd-watcher: %s\n", time_str, message);
    fclose(log_fp);
}

int main() {
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
