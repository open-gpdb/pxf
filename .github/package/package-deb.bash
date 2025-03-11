#!/usr/bin/env bash

function print_changelog() {
cat <<EOF
greenplum-pxf-6 (${PXF_PKG_VERSION}) stable; urgency=low

  * open-gpdb/pxf autobuild
-- ${BUILD_USER} <${BUILD_USER}@$(hostname)>  $(date +'%a, %d %b %Y %H:%M:%S %z')
EOF
}
