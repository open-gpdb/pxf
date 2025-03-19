download-xerces-c:
	wget --continue https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.3.0.tar.gz -P downloads

deb-pxf6-gpdb-bionic: download-xerces-c
	echo "===> Building PXF 6 for open-gpdb for bionic <==="
	docker build -f 'package/gpdb_bionic/Dockerfile'  .

deb-pxf6-gpdb-jammy: download-xerces-c
	echo "===> Building PXF 6 for open-gpdb for jammy <==="
	docker build -f 'package/gpdb_jammy/Dockerfile'  .

deb-pxf6-cbdb-jammy: download-xerces-c
	echo "===> Building PXF 6 for Cloudberry for jammy <==="
	docker build -f 'package/cbdb_jammy/Dockerfile'  .
