#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/demi.h"

// Simulate a demi_event structure
struct test_demi_event {
    char de_devname[256];
    int de_type;
};

// Simulate demi_read function with filtering
int test_demi_read(struct test_demi_event *de, const char *device_name, const char *action) {
    if (!de) {
        return -1;
    }
    
    // Simulate parsing device name and action
    strncpy(de->de_devname, device_name, sizeof(de->de_devname) - 1);
    de->de_devname[sizeof(de->de_devname) - 1] = '\0';
    
    if (strcmp(action, "add") == 0) {
        de->de_type = 1; // DEMI_ATTACH
    } else if (strcmp(action, "remove") == 0) {
        de->de_type = 2; // DEMI_DETACH
    } else if (strcmp(action, "change") == 0) {
        de->de_type = 3; // DEMI_CHANGE
    } else {
        de->de_type = 0; // DEMI_UNKNOWN
    }
    
    // Apply device filtering (this is what we added to demi_read)
    if (de->de_devname[0] != '\0' && !demi_is_device_allowed(de->de_devname)) {
        // Clear the device name to indicate this event should be ignored
        de->de_devname[0] = '\0';
    }
    
    return 0;
}

int main() {
    printf("=== Testing demi_read simulation with device filtering ===\n");
    
    // Set up the same filter as in the config file
    demi_set_allowed_devices("cd* vtbd* ada* md[0-3]");
    printf("Filter set to: cd* vtbd* ada* md[0-3]\n\n");
    
    struct test_demi_event de;
    const char *test_cases[][2] = {
        {"md0", "add"},
        {"md5", "add"},
        {"vtbd0", "remove"},
        {"sda0", "change"},
        {"cd0", "add"},
        {"ada0", "change"},
        {"hda0", "add"},
        {"md3", "remove"},
        {"md4", "add"}
    };
    
    printf("Testing device events:\n");
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const char *device = test_cases[i][0];
        const char *action = test_cases[i][1];
        
        if (test_demi_read(&de, device, action) == 0) {
            if (de.de_devname[0] == '\0') {
                printf("  %s (%s) -> FILTERED OUT (device not allowed)\n", device, action);
            } else {
                printf("  %s (%s) -> PROCESSED (device allowed)\n", device, action);
            }
        } else {
            printf("  %s (%s) -> ERROR\n", device, action);
        }
    }
    
    printf("\n=== Simulation test completed ===\n");
    return 0;
}
