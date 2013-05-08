#!/bin/bash

gdb --pid=$(cat /tmp/libbf-ncurses.pid) $@
 