FROM ubuntu:18.04

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

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
    /pxf_src/devops/packaging/deb/gpdb_bionic/debian/control

# install missing parts
RUN tar -xzf /pxf_src/downloads/xerces-c-3.1.1.tar.gz && \
    cd xerces-c-3.1.1 && \
    ./configure && \
    make -j$(nproc) > /dev/null && \
    make -j$(nproc) install

RUN cp -r /pxf_src/devops/packaging/deb/gpdb_bionic/debian /gpdb_src/

RUN chmod +x /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh && \
    chmod +x /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh && \
    chmod +x /pxf_src/devops/scripts/build-deb.sh

ENV SRC_DIR="/gpdb_src"
ENV BUILD_DESTINATION="/gpdb_src/debian/build"
ENV CONFIGURE_EXTRA_OPTS="--disable-gpperfmon --without-python --disable-orca --disable-yezzey"
WORKDIR /gpdb_src
RUN /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh
# we should run `/pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh`
# but it tries to build contribs as well... but we don't need it (and it fails to compile)
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8
RUN make -j$(nproc) \
     && make -j$(nproc) install
ENV GPDB_PKG_VERSION="6.0.0-bionic-dev"
RUN /pxf_src/devops/scripts/build-deb.sh

# get your packages in
# /greenplum-db-cb_1.0.0-dev_amd64.deb
# /greenplum-db-cb_1.0.0.tar.xz

RUN mkdir /build && \
    cp /*.deb /build && \
    cp /*.tar.* /build