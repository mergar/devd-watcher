#!/bin/sh
cc -DDEMI_PLATFORM_LINUX -DDEMI_LOCK_TIMEOUT_SECONDS=10 -Iinclude -Isrc/linux -o devd-watcher main.c src/linux/*.c -lpthread
