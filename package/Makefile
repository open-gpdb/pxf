.PHONY: help download-xerces-c-3-3-0 download-xerces-c-3-1-1 deb-gpdb-bionic deb-gpdb-jammy deb-cbdb-jammy deb-pxf6-gpdb-bionic deb-pxf6-gpdb-jammy deb-pxf6-cbdb-jammy

help:
	@echo
	@echo 'Possible targets'
	@echo	'  - deb-pxf6-gpdb-bionic - build PXF6 debian package for open-gpdb and place it in downloads'
	@echo	'  - deb-pxf6-gpdb-jammy - build PXF6 debian package for open-gpdb and place it in downloads'
	@echo	'  - deb-pxf6-cbdb-jammy - build PXF6 debian package for Cloudberry and place it in downloads'

download-xerces-c-3-3-0:
	wget --continue https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.3.0.tar.gz -P ../downloads/

# FIXME: does it works with 3.3.0 for all builds?
download-xerces-c-3-1-1:
	wget --continue https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.1.tar.gz -P ../downloads/

deb-gpdb-bionic: DOCKER_TAG = deb-gpdb-bionic
deb-gpdb-bionic: download-xerces-c-3-1-1
	echo "===> Building open-gpdb for bionic <==="
ifeq ($(shell test -f downloads/greenplum-db-6_6.0.0-bionic-dev_amd64.deb ; echo $$?),0)
	docker build -f 'gpdb_bionic/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-gpdb-bionic  ${DOCKER_TAG}:1.0
	docker cp extract-gpdb-bionic:/build/. ../downloads/
	docker rm extract-gpdb-bionic
else
	@echo "Skipping build: we already have gpdb bionic build in downloads."
endif

deb-gpdb-jammy: DOCKER_TAG = deb-gpdb-jammy
deb-gpdb-jammy: download-xerces-c-3-1-1
	echo "===> Building open-gpdb for jammy <==="
ifeq ($(shell test -f downloads/greenplum-db-6_6.0.0-jammy-dev_amd64.deb ; echo $$?),0)
	docker build -f 'gpdb_jammy/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-gpdb-jammy ${DOCKER_TAG}:1.0
	docker cp extract-gpdb-jammy:/build/. ../downloads/
	docker rm extract-gpdb-jammy
else
	@echo "Skipping build: we already have gpdb jammy build in downloads."
endif

deb-cbdb-jammy: DOCKER_TAG = deb-cbdb-jammy
deb-cbdb-jammy: download-xerces-c-3-3-0
	echo "===> Building Cloudberry for jammy <==="
ifeq ($(shell test -f downloads/greenplum-db-cb_1.0.0-dev_amd64.deb ; echo $$?),0)
	docker build -f 'cbdb_jammy/Dockerfile' . --tag ${DOCKER_TAG}:1.0
	docker create --name extract-cbdb-jammy ${DOCKER_TAG}:1.0
	docker cp extract-cbdb-jammy:/build/. ../downloads/
	docker rm extract-cbdb-jammy
else
	@echo "Skipping build: we already have cloudberry jammy build in downloads."
endif

#
# PXF deb builds
#
deb-pxf6-gpdb-bionic: DOCKER_TAG = deb-pxf6-gpdb-bionic
deb-pxf6-gpdb-bionic: deb-gpdb-bionic
	echo "===> Building PXF 6 for open-gpdb for bionic <==="
	docker build -f 'pxf_6_bionic/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-pxf6-gpdb-bionic  ${DOCKER_TAG}:1.0
	docker cp extract-pxf6-gpdb-bionic:/build/. ../downloads/
	docker rm extract-pxf6-gpdb-bionic


deb-pxf6-gpdb-jammy: DOCKER_TAG = deb-pxf6-gpdb-jammy
deb-pxf6-gpdb-jammy: deb-gpdb-jammy
	echo "===> Building PXF 6 for open-gpdb for jammy <==="
	docker build -f 'pxf_6_jammy/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-pxf6-gpdb-jammy  ${DOCKER_TAG}:1.0
	docker cp extract-pxf6-gpdb-jammy:/build/. ../downloads/
	docker rm extract-pxf6-gpdb-jammy

deb-pxf6-cbdb-jammy: DOCKER_TAG = deb-pxf6-cbdb-jammy
deb-pxf6-cbdb-jammy: deb-cbdb-jammy
	echo "===> Building PXF 6 for Cloudberry for jammy <==="
	docker build -f 'pxf_cb_jammy/Dockerfile' . --tag ${DOCKER_TAG}:1.0
	docker create --name extract-pxf6-cbdb-jammy  ${DOCKER_TAG}:1.0
	docker cp extract-pxf6-cbdb-jammy:/build/. ../downloads/
	docker rm extract-pxf6-cbdb-jammy