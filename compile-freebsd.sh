#!/bin/sh
cc -DDEMI_PLATFORM_FREEBSD -DDEMI_LOCK_TIMEOUT_SECONDS=10 -Iinclude -Isrc/freebsd -o devd-watcher main.c src/freebsd/*.c src/demi_filter.c -lpthread
