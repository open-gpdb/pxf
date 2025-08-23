FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND="noninteractive"

# Filesystem layout:
#   /pxf_src/downloads         - this repo's cache directory
#   /pxf_src/devops            - github/open-gpdb/gpdb-devops git submodule
#   /gpdb_src/                 - github/apache/cloudberry
#   /gpdb_src/debian           - one of the `debian` folders from github/open-gpdb/gpdb-devops
#   /gpdb_src/debian/build     - build destination for open-gpdb build

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        wget \
        git \
        python3 devscripts debhelper pbuilder reprepro equivs quilt

# checkout cloudberry to /gpdb_src
WORKDIR /
RUN git clone https://github.com/apache/cloudberry.git gpdb_src --single-branch --branch REL_2_STABLE --depth 1

# checkout this repository to /pxf_src
COPY . /pxf_src

# install CBDB build dependencies from debian/control
RUN mk-build-deps --build-dep \
    --install \
    --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
    /pxf_src/devops/packaging/deb/cbdb_jammy/debian/control

# install missing parts
RUN tar -xzf /pxf_src/downloads/xerces-c-3.3.0.tar.gz && \
    cd xerces-c-3.3.0 && \
    ./configure && \
    make -j$(nproc) > /dev/null && \
    make -j$(nproc) install

# FIXME: use https://github.com/apache/cloudberry-devops-release

RUN cp -r /pxf_src/devops/packaging/deb/cbdb_jammy/debian /gpdb_src/

WORKDIR /gpdb_src
ENV BUILD_DESTINATION=/gpdb_src/debian/build
RUN ./configure --without-perl --without-python --with-libxml --with-gssapi --disable-orca --prefix=${BUILD_DESTINATION} \
 && make -j$(nproc) > /dev/null

ENV GPDB_PKG_VERSION="1.0.0-dev"
RUN dpkg-buildpackage -us -uc
# get your packages in
# /greenplum-db-cb_1.0.0-dev_amd64.deb
# /greenplum-db-cb_1.0.0.tar.xz

RUN mkdir /build && \
    cp /*.deb /build

