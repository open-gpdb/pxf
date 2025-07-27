#!/bin/bash
set -e

# --------------------------------------------------------------------
# Run tests
# --------------------------------------------------------------------
cd /home/gpadmin/workspace/pxf/automation
make  # make without arguments runs all tests

# Keep container running
#tail -f /dev/null