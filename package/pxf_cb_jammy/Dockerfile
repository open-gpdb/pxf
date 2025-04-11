FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        wget \
        git \
        python3 devscripts debhelper pbuilder reprepro equivs quilt

# install golang
RUN wget -O - "https://go.dev/dl/go1.22.9.linux-amd64.tar.gz" | tar -C /usr/local -xz

# install greenplum:
COPY . /pxf_src/
RUN apt -o Apt::Get::Assume-Yes=true install /pxf_src/downloads/greenplum-db-cb_1.0.0-dev_amd64.deb

#
# build and package PXF
#

# add unicode support
RUN locale-gen en_US.utf8

# copy source & create layout for packaging
ENV PXF_PKG_VERSION "1.0.0-jammy-dev"
COPY . /packaging/
RUN cp -r /pxf_src/devops/packaging/deb/pxf_cb_jammy/debian /pxf_src/debian \
 && source /pxf_src/package/package-deb.bash \
 && print_cb_changelog > /pxf_src/debian/changelog

# pxf will install binaries into GPHOME
# but dpkg-buildpackage will not get files from /usr/cloudberry-db
RUN mkdir -p  /pxf_src/debian/build \
 && ln -s /usr/cloudberry-db /pxf_src/debian/build/gp

WORKDIR /pxf_src/
# install PXF build dependencies from debian/control
RUN mk-build-deps  --build-dep --install --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' debian/control


# dpkg-buildpackage will call make && make install (via debian/rules)
RUN source /usr/cloudberry-db/greenplum_path.sh \
 && PATH="/usr/local/go/bin:$PATH" \
    GPHOME="/usr/cloudberry-db" \
    PXF_HOME="/pxf_src/debian/build/gp/pxf" \
    JAVA_HOME="/usr/lib/jvm/java-1.11.0-openjdk-amd64" \
    dpkg-buildpackage -us -uc

# get your packages in
# /greenplum-pxf-1_6.10.1_amd64.deb
RUN mkdir /build && \
    cp /*.deb /build