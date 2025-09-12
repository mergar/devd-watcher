#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct config {
    char *lock_dir;
    int lock_timeout_seconds;
    char *allowed_devices;
};

static struct config g_config = {
    .lock_dir = NULL,
    .lock_timeout_seconds = 5,
    .allowed_devices = NULL
};

static void trim_whitespace(char *str) {
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    char *start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static int parse_config_file(const char *config_path) {
    FILE *file = fopen(config_path, "r");
    if (!file) {
        return -1;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        // Find the '=' character
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }

        // Split the line into key and value
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        // Trim whitespace
        trim_whitespace(key);
        trim_whitespace(value);

        // Remove quotes if present
        if (value[0] == '"' && value[strlen(value) - 1] == '"') {
            value[strlen(value) - 1] = '\0';
            value++;
        }

        // Parse configuration parameters
        if (strcmp(key, "DEMI_LOCK_DIR") == 0) {
            free(g_config.lock_dir);
            g_config.lock_dir = strdup(value);
        } else if (strcmp(key, "DEMI_LOCK_TIMEOUT_SECONDS") == 0) {
            g_config.lock_timeout_seconds = atoi(value);
            if (g_config.lock_timeout_seconds <= 0) {
                g_config.lock_timeout_seconds = 5;
            }
        } else if (strcmp(key, "DEMI_ALLOWED_DEVICES") == 0) {
            free(g_config.allowed_devices);
            g_config.allowed_devices = strdup(value);
        }
    }

    fclose(file);
    return 0;
}

static void cleanup_config(void) {
    free(g_config.lock_dir);
    free(g_config.allowed_devices);
}

static int is_device_allowed(const char *devname) {
    if (!g_config.allowed_devices || strlen(g_config.allowed_devices) == 0) {
        return 1; // Allow all devices if no filter is set
    }

    char *allowed_copy = strdup(g_config.allowed_devices);
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

static void print_usage(const char *progname) {
    printf("Usage: %s [-c config_file] [-t test_device]\n", progname);
    printf("  -c config_file  Configuration file path (default: etc/devd-watcher.conf)\n");
    printf("  -t test_device  Test device name against allowed devices filter\n");
    printf("  -h              Show this help message\n");
}

int main(int argc, char *argv[])
{
    const char *config_file = "etc/devd-watcher.conf";
    const char *test_device = NULL;
    int opt;

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "c:t:h")) != -1) {
        switch (opt) {
            case 'c':
                config_file = optarg;
                break;
            case 't':
                test_device = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    printf("=== devd-watcher Final Configuration Test ===\n");
    printf("Config file: %s\n", config_file);

    // Parse configuration file
    if (parse_config_file(config_file) == -1) {
        printf("Warning: Could not read config file '%s': %s\n", config_file, strerror(errno));
        printf("Using default configuration\n");
    } else {
        printf("Configuration loaded successfully!\n");
    }

    // Register cleanup function
    atexit(cleanup_config);

    // Print loaded configuration
    printf("\n=== Loaded Configuration ===\n");
    printf("DEMI_LOCK_DIR: %s\n", g_config.lock_dir ? g_config.lock_dir : "(default)");
    printf("DEMI_LOCK_TIMEOUT_SECONDS: %d\n", g_config.lock_timeout_seconds);
    printf("DEMI_ALLOWED_DEVICES: %s\n", g_config.allowed_devices ? g_config.allowed_devices : "(all devices allowed)");

    // Test device filtering if device name provided
    if (test_device) {
        printf("\n=== Device Filter Test ===\n");
        printf("Testing device: %s\n", test_device);
        if (is_device_allowed(test_device)) {
            printf("Result: ALLOWED\n");
        } else {
            printf("Result: BLOCKED\n");
        }
    } else {
        // Test all common device patterns
        printf("\n=== Device Filter Test (All Patterns) ===\n");
        const char *test_devices[] = {
            "cd0", "cd1", "vtbd0", "vtbd1", "ada0", "ada1", 
            "md0", "md1", "md2", "md3", "md4", "md5",
            "sda0", "sdb0", "hda0", "hdb0"
        };
        
        for (int i = 0; i < sizeof(test_devices) / sizeof(test_devices[0]); i++) {
            printf("Device %s -> %s\n", test_devices[i], 
                   is_device_allowed(test_devices[i]) ? "ALLOWED" : "BLOCKED");
        }
    }

    printf("\n=== Test completed successfully ===\n");
    return EXIT_SUCCESS;
}
