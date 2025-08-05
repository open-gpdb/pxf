FROM ubuntu:18.04

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND=noninteractive

# Filesystem layout:
#   /pxf_src/downloads         - this repo's cache directory
#   /pxf_src/devops            - github/open-gpdb/gpdb-devops git submodule
#   /gpdb_src/                 - github/open-gpdb/gpdb
#   /gpdb_src/debian           - one of the `debian` folders from github/open-gpdb/gpdb-devops
#   /gpdb_src/debian/build     - build destination for open-gpdb build

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        python3 devscripts debhelper pbuilder reprepro equivs quilt \
        sudo wget git locales

# checkout greenplum to /gpdb_src
WORKDIR /
RUN git clone https://github.com/open-gpdb/gpdb.git gpdb_src --single-branch --branch OPENGPDB_STABLE --depth 1

# checkout this repository to /pxf_src
COPY . /pxf_src/

# install GPDB build dependencies from debian/control
RUN mk-build-deps --build-dep \
    --install \
    --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
    /pxf_src/devops/packaging/open-gpdb/deb/18.04/control

# install missing parts
RUN cd /tmp/ && \
    git clone https://github.com/greenplum-db/gp-xerces-archive.git && \
    cd /tmp/gp-xerces-archive/ && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) > /dev/null && \
    make -j$(nproc) install

RUN cd /tmp/ && \
    git clone https://github.com/boundary/sigar.git -b master && \
    cd /tmp/sigar/ && \
    cmake . && make && \
    make install

RUN mkdir -p /gpdb_src/debian && \
    cp -r /pxf_src/devops/packaging/open-gpdb/deb/18.04/* /gpdb_src/debian

RUN chmod +x /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh && \
    chmod +x /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh && \
    chmod +x /pxf_src/devops/build_automation/gpdb/scripts/vendor.sh && \
    chmod +x /pxf_src/devops/scripts/build-deb.sh

ENV SRC_DIR="/gpdb_src"
ENV BUILD_DESTINATION="/gpdb_src/debian/build"
ENV CONFIGURE_EXTRA_OPTS="--disable-gpperfmon --without-python --enable-orca --disable-yezzey"
WORKDIR /gpdb_src
RUN /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh
RUN /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh
RUN /pxf_src/devops/build_automation/gpdb/scripts/vendor.sh
ENV GPDB_PKG_VERSION="6.0.0-bionic-dev"
RUN /pxf_src/devops/scripts/build-deb.sh && \
  cp /usr/local/lib/libxerces* ${BUILD_DESTINATION}/lib

# get your packages in
# /greenplum-db-cb_1.0.0-dev_amd64.deb
# /greenplum-db-cb_1.0.0.tar.xz

RUN mkdir /build && \
    cp /*.deb /build