FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
    ca-certificates \
    build-essential \
    locales \
    wget \
    git

# install golang
RUN wget -O - "https://go.dev/dl/go1.22.9.linux-amd64.tar.gz" | tar -C /usr/local -xz

# install JDK
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    openjdk-11-jdk-headless

# https://github.com/yezzey-gp/ygp/blob/YGP_6.27_STABLE/README.ubuntu.bash (kind of)
RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    bison \
    ccache \
    cmake \
    curl \
    flex \
    git-core \
    gcc \
    g++ \
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
    python3-dev \
    python3-pip \
    python3-psycopg2 \
    python3-psutil \
    python3-yaml \
    python3-pygresql \
    zlib1g-dev \
    libpstreams-dev \
    libxerces-c-dev

RUN apt-get install --yes --no-install-recommends --no-install-suggests \
    openssh-server openssh-client \
    iputils-ping \
    locales \
    python3-dev \
    pkg-config \
    rsync


WORKDIR /usr/local

RUN locale-gen en_US.utf8 \
 && apt-get install --yes --no-install-recommends --no-install-suggests \
    libxerces-c-dev

# Fetch sources:
#RUN git clone https://github.com/apache/cloudberry.git /usr/local/gpdb_src -b 1.6.0 --depth 1 --single-branch
RUN wget -qO-  https://github.com/apache/cloudberry/archive/refs/tags/1.6.0.tar.gz | tar xz \
    && mv cloudberry-1.6.0 /usr/local/gpdb_src

COPY . /usr/local/pxf_src


WORKDIR /usr/local/gpdb_src
RUN ./configure --without-perl --without-python --with-libxml --with-gssapi --disable-orca --prefix=/opt/greenplum-db-cb > /dev/null \
 && make -j$(nproc) > /dev/null \
 && make install \
 && echo "/usr/local/lib" >> /etc/ld.so.conf && ldconfig

# Build PXF:
WORKDIR /usr/local/pxf_src
RUN source /opt/greenplum-db-cb/greenplum_path.sh \
 && export PATH="/usr/local/go/bin:$PATH" \
 && make extensions