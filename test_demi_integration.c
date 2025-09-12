#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/demi.h"

int main() {
    printf("=== Testing demi device filtering integration ===\n");
    
    // Test 1: No filter set (should allow all devices)
    printf("\n1. Testing with no filter (should allow all):\n");
    demi_set_allowed_devices(NULL);
    printf("  md0 -> %s\n", demi_is_device_allowed("md0") ? "ALLOWED" : "BLOCKED");
    printf("  sda0 -> %s\n", demi_is_device_allowed("sda0") ? "ALLOWED" : "BLOCKED");
    printf("  cd0 -> %s\n", demi_is_device_allowed("cd0") ? "ALLOWED" : "BLOCKED");
    
    // Test 2: Filter with wildcards
    printf("\n2. Testing with wildcard filter 'md* cd*':\n");
    demi_set_allowed_devices("md* cd*");
    printf("  md0 -> %s\n", demi_is_device_allowed("md0") ? "ALLOWED" : "BLOCKED");
    printf("  md5 -> %s\n", demi_is_device_allowed("md5") ? "ALLOWED" : "BLOCKED");
    printf("  cd0 -> %s\n", demi_is_device_allowed("cd0") ? "ALLOWED" : "BLOCKED");
    printf("  sda0 -> %s\n", demi_is_device_allowed("sda0") ? "ALLOWED" : "BLOCKED");
    printf("  vtbd0 -> %s\n", demi_is_device_allowed("vtbd0") ? "ALLOWED" : "BLOCKED");
    
    // Test 3: Filter with range pattern
    printf("\n3. Testing with range filter 'md[0-3]':\n");
    demi_set_allowed_devices("md[0-3]");
    printf("  md0 -> %s\n", demi_is_device_allowed("md0") ? "ALLOWED" : "BLOCKED");
    printf("  md1 -> %s\n", demi_is_device_allowed("md1") ? "ALLOWED" : "BLOCKED");
    printf("  md2 -> %s\n", demi_is_device_allowed("md2") ? "ALLOWED" : "BLOCKED");
    printf("  md3 -> %s\n", demi_is_device_allowed("md3") ? "ALLOWED" : "BLOCKED");
    printf("  md4 -> %s\n", demi_is_device_allowed("md4") ? "ALLOWED" : "BLOCKED");
    printf("  md5 -> %s\n", demi_is_device_allowed("md5") ? "ALLOWED" : "BLOCKED");
    
    // Test 4: Complex filter from config file
    printf("\n4. Testing with complex filter 'cd* vtbd* ada* md[0-3]':\n");
    demi_set_allowed_devices("cd* vtbd* ada* md[0-3]");
    const char *test_devices[] = {
        "cd0", "cd1", "vtbd0", "vtbd1", "ada0", "ada1", 
        "md0", "md1", "md2", "md3", "md4", "md5",
        "sda0", "sdb0", "hda0", "hdb0"
    };
    
    for (int i = 0; i < sizeof(test_devices) / sizeof(test_devices[0]); i++) {
        printf("  %s -> %s\n", test_devices[i], 
               demi_is_device_allowed(test_devices[i]) ? "ALLOWED" : "BLOCKED");
    }
    
    // Test 5: Empty device name
    printf("\n5. Testing edge cases:\n");
    printf("  Empty string -> %s\n", demi_is_device_allowed("") ? "ALLOWED" : "BLOCKED");
    printf("  NULL -> %s\n", demi_is_device_allowed(NULL) ? "ALLOWED" : "BLOCKED");
    
    printf("\n=== Integration test completed ===\n");
    return 0;
}
