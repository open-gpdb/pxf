#!/bin/bash

function log() {
    echo "=====> $(date) $* <======" >> "${PXF_LOGDIR}/pxf-oom.log"
}

function _main() {
    sleep 1
    log 'Stopping PXF'
    "${PXF_HOME}/bin/pxf" stop && return
    # "insurance policy":
    local PID=$1
    if [[ -n $PID ]]; then
        ps "$1" >/dev/null && kill -9 "$1"
    fi
}

# Close inherited JVM FDs before running child commands.
# Parse the block first and exit inside it because this also closes Bash's script FD.
{
    for fd in /dev/fd/[0-9]*; do
        fd=${fd##*/}
        [[ $fd =~ ^[0-9]+$ && $fd -gt 2 ]] || continue
        eval "exec ${fd}>&-"
    done

    log 'PXF Out of memory detected'
    _main "$1" &
    log 'PXF shutdown scheduled'
    exit
}
