#!/bin/bash
set -e
set -x

# if group name is passed as an argument, run make with GROUP parameter
# otherwise run make without arguments (runs all tests)
cd /home/gpadmin/workspace/pxf/automation
if [ -n "$1" ]; then
    make GROUP="$1"
else
    make
fi