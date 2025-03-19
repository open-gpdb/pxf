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
RUN wget -c https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.1.tar.gz -O - | tar -xz \
 && cd xerces-c-3.1.1 && ./configure && make -j$(nproc) > /dev/null && make -j$(nproc) install


RUN chmod +x /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh && \
    chmod +x /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh && \
    chmod +x /pxf_src/devops/scripts/build-deb.sh

ENV SRC_DIR="/gpdb_src"
ENV BUILD_DESTINATION="/pxf_src/devops/packaging/deb/jammy/debian/build"
ENV CONFIGURE_EXTRA_OPTS="--disable-gpperfmon"
WORKDIR /gpdb_src
RUN /pxf_src/devops/build_automation/gpdb/scripts/configure-gpdb.sh
RUN /pxf_src/devops/build_automation/gpdb/scripts/build-gpdb.sh
RUN /pxf_src/devops/scripts/build-deb.sh

## install greenplum for jammy:
#WORKDIR /root/
#RUN wget https://github.com/open-gpdb/gpdb/releases/download/6.28.2/greenplum-db-6_28.1.0--yandex.1.59a66a1_amd64.deb \
# && apt -o Apt::Get::Assume-Yes=true install ./greenplum-db-6_28.1.0--yandex.1.59a66a1_amd64.deb
#
##
## build and package PXF
##
#
## add unicode support
#RUN locale-gen en_US.utf8
#
## copy source & create layout for packaging
#ENV PXF_PKG_VERSION 6.10.1
#COPY . /packaging/
#RUN git clone https://github.com/open-gpdb/gpdb-devops.git /packaging/gpdb-devops --single-branch --branch master --depth 1 \
# && cp -r /packaging/gpdb-devops/packaging/deb/pxf_6_jammy/debian /packaging/debian \
# && cp /packaging/.github/package/package-deb.bash /packaging/ \
# && source /packaging/package-deb.bash \
# && print_changelog > /packaging/debian/changelog
#
## pxf will install binaries into GPHOME
## but dpkg-buildpackage will not get files from /opt/greenplum-db-6
#RUN mkdir -p  /packaging/debian/build \
# && ln -s /opt/greenplum-db-6 /packaging/debian/build/gp
#
#WORKDIR /packaging/
#
#
## dpkg-buildpackage will call make && make install (via debian/rules)
#RUN source /opt/greenplum-db-6/greenplum_path.sh \
# && PATH="/usr/local/go/bin:$PATH" \
#    GPHOME="/opt/greenplum-db-6" \
#    PXF_HOME="/opt/greenplum-pxf-6" \
#    JAVA_HOME="/usr/lib/jvm/java-1.11.0-openjdk-amd64" \
#    dpkg-buildpackage -us -uc