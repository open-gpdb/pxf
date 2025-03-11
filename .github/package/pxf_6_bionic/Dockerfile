FROM ubuntu:18.04

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update

# FIXME: what about locales?
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    ca-certificates \
    build-essential \
    locales \
    wget \
    git

# Install golang
RUN wget -O - "https://go.dev/dl/go1.22.9.linux-amd64.tar.gz" | tar -C /usr/local -xz

# install JDK
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    openjdk-11-jdk-headless

# Install Greenplum 6.x (no deb files available - build from source)
# https://github.com/open-gpdb/gpdb/blob/OPENGPDB_STABLE/README.ubuntu.bash (kind of)
RUN apt-get install  --yes --no-install-recommends --no-install-suggests \
      bison \
      ccache \
      cmake \
      curl \
      flex \
      git-core \
      gcc-8 \
      g++-8 \
      inetutils-ping \
      krb5-kdc \
      krb5-admin-server \
      libapr1-dev \
      libbz2-dev \
      libcurl4-gnutls-dev \
      libevent-dev \
      libkrb5-dev \
      libpam-dev \
      libperl-dev \
      libreadline-dev \
      libssl-dev \
      libxml2-dev \
      libyaml-dev \
      libzstd-dev \
      locales \
      net-tools \
      ninja-build \
      openssh-client \
      openssh-server \
      openssl \
      python-dev \
      python-pip \
      python3-pip \
      python-psutil \
      python-yaml \
      zlib1g-dev \
      libpstreams-dev \
      python-setuptools \
  && pip install conan

WORKDIR /usr/local/xerces_src
RUN locale-gen en_US.utf8 \
 && wget -c https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.1.tar.gz -O - | tar -xz \
 && cd xerces-c-3.1.1 && ./configure && make -j$(nproc) > /dev/null && make -j$(nproc) install

# fetch & build greenplum
WORKDIR /usr/local
RUN git clone https://github.com/open-gpdb/gpdb.git gpdb_src --single-branch --branch 6.25.1 --depth 1

WORKDIR /usr/local/gpdb_src
RUN ./configure --without-perl --without-python --with-libxml --with-gssapi --disable-orca --prefix=/opt/greenplum-db-6 > /dev/null \
 && make -j$(nproc) > /dev/null \
 && make -j$(nproc) install > /dev/null \
 && echo "/usr/local/lib" >> /etc/ld.so.conf && ldconfig

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
 && cp /packaging/.github/package/package-deb.bash      /packaging/ \
 && source /packaging/package-deb.bash \
 && print_changelog > /packaging/debian/changelog

# pxf will install binaries into GPHOME
# but dpkg-buildpackage will not get files from /opt/greenplum-db-6
RUN mkdir -p  /packaging/debian/build \
 && ln -s /opt/greenplum-db-6 /packaging/debian/build/gp


# dpkg-buildpackage will call make && make install (via debian/rules)
RUN source /opt/greenplum-db-6/greenplum_path.sh \
 && PATH="/usr/local/go/bin:$PATH" \
    GPHOME="/opt/greenplum-db-6" \
    PXF_HOME="/opt/greenplum-pxf-6" \
    JAVA_HOME="/usr/lib/jvm/java-1.11.0-openjdk-amd64" \
    dpkg-buildpackage -us -uc