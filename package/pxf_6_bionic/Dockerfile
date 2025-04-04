FROM ubuntu:18.04

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update &&\
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        build-essential \
        wget \
        git \
        python3 devscripts debhelper pbuilder reprepro equivs

# Install golang
RUN wget -O - "https://go.dev/dl/go1.22.9.linux-amd64.tar.gz" | tar -C /usr/local -xz

COPY . /pxf_src/
RUN apt -o Apt::Get::Assume-Yes=true install /pxf_src/downloads/greenplum-db-6_6.0.0-bionic-dev_amd64.deb

#
# build and package PXF
#

# add unicode support
RUN locale-gen en_US.utf8

# copy source & create layout for packaging
ENV PXF_PKG_VERSION "6.10.1-jammy-dev"
RUN cp -r /pxf_src/devops/packaging/deb/pxf_6_bionic/debian /pxf_src/debian \
 && source /pxf_src/package/package-deb.bash \
 && print_changelog > /pxf_src/debian/changelog

# pxf will install binaries into GPHOME
# but dpkg-buildpackage will not get files from /opt/greenplum-db-6
RUN mkdir -p  /pxf_src/debian/build \
 && ln -s /opt/greenplum-db-6 /pxf_src/debian/build/gp

WORKDIR /pxf_src/
# install PXF build dependencies from debian/control
RUN mk-build-deps  --build-dep --install --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' debian/control

# dpkg-buildpackage will call make && make install (via debian/rules)
RUN source /opt/greenplum-db-6/greenplum_path.sh \
 && PATH="/usr/local/go/bin:$PATH" \
    GPHOME="/opt/greenplum-db-6" \
    PXF_HOME="/opt/greenplum-pxf-6" \
    JAVA_HOME="/usr/lib/jvm/java-1.11.0-openjdk-amd64" \
    dpkg-buildpackage -us -uc

# get your packages in
# /greenplum-pxf-6_6.10.1_amd64.deb
RUN mkdir /build && \
    cp /*.deb /build && \
    cp /*.tar.* /build