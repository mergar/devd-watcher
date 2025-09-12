#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/demi.h"

static char *g_allowed_devices = NULL;

void demi_set_allowed_devices(const char *allowed_devices) {
    free(g_allowed_devices);
    if (allowed_devices && strlen(allowed_devices) > 0) {
        g_allowed_devices = strdup(allowed_devices);
    } else {
        g_allowed_devices = NULL;
    }
}

int demi_is_device_allowed(const char *devname) {
    if (!g_allowed_devices || strlen(g_allowed_devices) == 0) {
        return 1; // Allow all devices if no filter is set
    }

    if (!devname || strlen(devname) == 0) {
        return 0; // Block empty device names
    }

    char *allowed_copy = strdup(g_allowed_devices);
    if (!allowed_copy) {
        return 1; // Allow all on memory error
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
                } else {
                    // Pattern like "md[0-3]suffix" - check prefix and suffix
                    if (strstr(devname + prefix_len, suffix) != NULL) {
                        allowed = 1;
                        break;
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
