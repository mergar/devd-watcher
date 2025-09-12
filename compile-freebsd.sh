#!/bin/sh
cc -DDEMI_PLATFORM_FREEBSD -Ddemi_lock_timeout_seconds=10 -Iinclude -Isrc/freebsd -o devd-watcher main.c src/freebsd/*.c -lpthread
