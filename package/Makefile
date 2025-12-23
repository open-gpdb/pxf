.PHONY: help download-xerces-c-3-3-0 deb-gpdb-bionic deb-gpdb-jammy deb-pxf6-gpdb-bionic deb-pxf6-gpdb-jammy

help:
	@echo
	@echo 'Possible targets'
	@echo	'  - deb-pxf6-gpdb-bionic - build PXF6 debian package for open-gpdb and place it in downloads'
	@echo	'  - deb-pxf6-gpdb-jammy - build PXF6 debian package for open-gpdb and place it in downloads'

download-xerces-c-3-3-0:
	wget --continue https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.3.0.tar.gz -P ../downloads/

deb-gpdb-bionic: DOCKER_TAG = deb-gpdb-bionic
deb-gpdb-bionic:
	echo "===> Building open-gpdb for bionic <==="
ifeq ($(shell test -f ../downloads/greenplum-db-6_6.0.0-bionic-dev_amd64.deb ; echo $$?),0)
	@echo "Skipping build: we already have gpdb bionic build in downloads."
else
	docker build -f 'gpdb_bionic/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-gpdb-bionic  ${DOCKER_TAG}:1.0
	docker cp extract-gpdb-bionic:/build/. ../downloads/
	docker rm extract-gpdb-bionic
endif

deb-gpdb-jammy: DOCKER_TAG = deb-gpdb-jammy
deb-gpdb-jammy:
	echo "===> Building open-gpdb for jammy <==="
ifeq ($(shell test -f ../downloads/greenplum-db-6_6.0.0-jammy-dev_amd64.deb ; echo $$?),0)
	@echo "Skipping build: we already have gpdb jammy build in downloads."
else
	docker build -f 'gpdb_jammy/Dockerfile' .. --tag ${DOCKER_TAG}:1.0
	docker create --name extract-gpdb-jammy ${DOCKER_TAG}:1.0
	docker cp extract-gpdb-jammy:/build/. ../downloads/
	docker rm extract-gpdb-jammy
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
