#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *allowed_devices = "cd* vtbd* ada* md[0-3]";

static int is_device_allowed(const char *devname) {
    if (!allowed_devices || strlen(allowed_devices) == 0) {
        return 1; // Allow all devices if no filter is set
    }

    char *allowed_copy = strdup(allowed_devices);
    if (!allowed_copy) {
        return 1; // Allow all on memory error
    }

    char *token = strtok(allowed_copy, " ");
    int allowed = 0;
    while (token != NULL) {
        printf("  Checking pattern: '%s' against device: '%s'", token, devname);
        
        // Simple wildcard matching for patterns like "cd*", "vtbd*", etc.
        size_t pattern_len = strlen(token);
        if (pattern_len > 0 && token[pattern_len - 1] == '*') {
            // Remove the '*' and compare prefix
            token[pattern_len - 1] = '\0';
            if (strncmp(devname, token, pattern_len - 1) == 0) {
                printf(" -> MATCH (prefix)\n");
                allowed = 1;
                break;
            } else {
                printf(" -> no match (prefix)\n");
            }
        } else {
            // Exact match
            if (strcmp(devname, token) == 0) {
                printf(" -> MATCH (exact)\n");
                allowed = 1;
                break;
            } else {
                printf(" -> no match (exact)\n");
            }
        }
        token = strtok(NULL, " ");
    }

    free(allowed_copy);
    return allowed;
}

int main() {
    const char *test_devices[] = {
        "cd0", "cd1", "vtbd0", "vtbd1", "ada0", "ada1", 
        "md0", "md1", "md2", "md3", "md4", "md5",
        "sda0", "sdb0", "hda0", "hdb0"
    };
    
    printf("=== Device Filter Test ===\n");
    printf("Allowed devices pattern: %s\n\n", allowed_devices);
    
    for (int i = 0; i < sizeof(test_devices) / sizeof(test_devices[0]); i++) {
        printf("Testing device: %s\n", test_devices[i]);
        if (is_device_allowed(test_devices[i])) {
            printf("  Result: ALLOWED\n");
        } else {
            printf("  Result: BLOCKED\n");
        }
        printf("\n");
    }
    
    return 0;
}
