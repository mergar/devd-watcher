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

// Simple device filtering function for testing
int demi_is_device_allowed(const char *devname) {
    const char *allowed_devices = "cd* vtbd* ada* md[0-3]";
    
    if (!devname || strlen(devname) == 0) {
        return 0;
    }

    char *allowed_copy = strdup(allowed_devices);
    if (!allowed_copy) {
        return 1;
    }

    char *token = strtok(allowed_copy, " ");
    int allowed = 0;
    while (token != NULL) {
        // Check for bracket patterns like "md[0-3]"
        char *bracket_start = strchr(token, '[');
        char *bracket_end = strchr(token, ']');
        
        if (bracket_start && bracket_end && bracket_end > bracket_start) {
            // Extract prefix before bracket
            size_t prefix_len = bracket_start - token;
            if (prefix_len > 0 && strncmp(devname, token, prefix_len) == 0) {
                // Extract the suffix after the bracket pattern
                char *suffix = bracket_end + 1;
                if (strlen(suffix) == 0) {
                    // Pattern like "md[0-3]" - check if suffix matches range
                    char *range = bracket_start + 1;
                    *bracket_end = '\0'; // Terminate range string
                    
                    // Simple range parsing for single digits like "0-3"
                    if (strlen(range) == 3 && range[1] == '-') {
                        char start_char = range[0];
                        char end_char = range[2];
                        char dev_suffix = devname[prefix_len];
                        
                        if (dev_suffix >= start_char && dev_suffix <= end_char) {
                            allowed = 1;
                            break;
                        }
                    }
                }
            }
        } else {
            // Simple wildcard matching for patterns like "cd*", "vtbd*", etc.
            size_t pattern_len = strlen(token);
            if (pattern_len > 0 && token[pattern_len - 1] == '*') {
                // Remove the '*' and compare prefix
                token[pattern_len - 1] = '\0';
                if (strncmp(devname, token, pattern_len - 1) == 0) {
                    allowed = 1;
                    break;
                }
            } else {
                // Exact match
                if (strcmp(devname, token) == 0) {
                    allowed = 1;
                    break;
                }
            }
        }
        token = strtok(NULL, " ");
    }

    free(allowed_copy);
    return allowed;
}

int main() {
    printf("Testing demi logging with device filtering...\n");
    
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
