#!/usr/bin/env bash

# Load settings
root=`cd \`dirname $0\`/..;pwd`
bin=${root}/bin
. ${bin}/gphd-env.sh

cluster_initialized
if [ $? -ne 0 ] && [ ! $PXFDEMO ]; then
	echo cluster not initialized
	echo please run ${bin}/init-gphd.sh
	exit 1
fi

# Start PXF
pushd ${GPHD_ROOT}
for (( i=0; i < ${SLAVES}; i++ ))
do
	${bin}/pxf-service.sh start ${i} | sed "s/^/node $i: /"
done
popd
