FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        python3 devscripts debhelper pbuilder reprepro equivs \
        sudo wget git

# checkout greenplum to /gpdb_src
WORKDIR /
RUN git clone https://github.com/open-gpdb/gpdb.git gpdb_src --single-branch --branch 6.28.2 --depth 1

# checkout this repository to /pxf_src
COPY . /pxf_src/

# install GPDB build dependencies from debian/control
RUN mk-build-deps --build-dep \
    --install \
    --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
    /pxf_src/devops/packaging/deb/jammy/debian/control

# install missing parts
RUN tar -xzf /pxf_src/downloads/xerces-c-3.3.0.tar.gz && \
    cd xerces-c-3.3.0 && \
    ./configure && \
    make -j$(nproc) > /dev/null && \
    make -j$(nproc) install


RUN chmod +x /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh && \
    chmod +x /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh && \
    chmod +x /pxf_src/devops/scripts/build-deb.sh

ENV SRC_DIR="/gpdb_src"
ENV BUILD_DESTINATION="/pxf_src/devops/packaging/deb/jammy/debian/build"
ENV CONFIGURE_EXTRA_OPTS="--disable-gpperfmon --without-python --disable-orca"
WORKDIR /gpdb_src
RUN /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh
RUN /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh
RUN /pxf_src/devops/scripts/build-deb.sh
