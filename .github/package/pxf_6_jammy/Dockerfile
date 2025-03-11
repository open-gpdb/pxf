FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update

# networking
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    ca-certificates \
    wget \
    git

# install golang
RUN wget -O - "https://go.dev/dl/go1.22.9.linux-amd64.tar.gz" | tar -C /usr/local -xz

# install JDK
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    openjdk-11-jdk-headless

# install PXF build dependencies:
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    build-essential \
    locales \
    libcurl4-gnutls-dev

# install greenplum for jammy:
WORKDIR /root/
RUN wget https://github.com/open-gpdb/gpdb/releases/download/6.28.2/greenplum-db-6_28.1.0--yandex.1.59a66a1_amd64.deb \
 && apt -o Apt::Get::Assume-Yes=true install ./greenplum-db-6_28.1.0--yandex.1.59a66a1_amd64.deb

#
# build and package PXF
#

# add unicode support
RUN locale-gen en_US.utf8
# install devscripts
RUN apt-get install -y python3 devscripts debhelper pbuilder reprepro equivs

# copy source & create layout for packaging
ENV PXF_PKG_VERSION 6.10.1
COPY . /packaging/
RUN cp -r /packaging/.github/package/pxf_6_jammy/debian /packaging/debian \
 && cp /packaging/.github/package/package-deb.bash   /packaging/ \
 && source /packaging/package-deb.bash \
 && print_changelog > /packaging/debian/changelog

# pxf will install binaries into GPHOME
# but dpkg-buildpackage will not get files from /opt/greenplum-db-6
RUN mkdir -p  /packaging/debian/build \
 && ln -s /opt/greenplum-db-6 /packaging/debian/build/gp

WORKDIR /packaging/
# install PXF build dependencies from debian/control
RUN mk-build-deps  --build-dep --install --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' debian/control


# dpkg-buildpackage will call make && make install (via debian/rules)
RUN source /opt/greenplum-db-6/greenplum_path.sh \
 && PATH="/usr/local/go/bin:$PATH" \
    GPHOME="/opt/greenplum-db-6" \
    PXF_HOME="/opt/greenplum-pxf-6" \
    JAVA_HOME="/usr/lib/jvm/java-1.11.0-openjdk-amd64" \
    dpkg-buildpackage -us -uc