FROM ubuntu:jammy

SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND noninteractive

# networking
RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
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
RUN wget https://github.com/open-gpdb/gpdb/releases/download/6.6.6-tmptag/greenplum-db-6_6.0.0--yandex.1.a09da17_amd64.deb \
 && apt -o Apt::Get::Assume-Yes=true install ./greenplum-db-6_6.0.0--yandex.1.a09da17_amd64.deb

# add unicode support
RUN locale-gen en_US.utf8

# copy source & build PXF:
COPY . /usr/local/pxf_src
WORKDIR /usr/local/pxf_src
RUN source /opt/greenplum-db-6/greenplum_path.sh \
 && export PATH="/usr/local/go/bin:$PATH" \
 && make install
