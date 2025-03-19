FROM ubuntu:jammy

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install --yes --no-install-recommends --no-install-suggests \
        ca-certificates \
        wget \
        git \
        python3 devscripts debhelper pbuilder reprepro equivs

# checkout cloudberry to /gpdb_src
WORKDIR /
RUN wget -qO-  https://github.com/apache/cloudberry/archive/refs/tags/1.6.0.tar.gz | tar xz \
    && mv cloudberry-1.6.0 /gpdb_src

# checkout this repository to /pxf_src
COPY . /pxf_src

# install CBDB build dependencies from debian/control
RUN mk-build-deps --build-dep \
    --install \
    --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
    /pxf_src/devops/packaging/deb/cbdb_jammy/debian/control

# install missing parts
RUN tar -xzf /pxf_src/downloads/xerces-c-3.3.0.tar.gz && \
    cd xerces-c-3.1.1 && \
    ./configure && \
    make -j$(nproc) > /dev/null && \
    make -j$(nproc) install

# FIXME: use https://github.com/apache/cloudberry-devops-release

# create folder structure:
RUN cp -rv /pxf_src/devops/packaging/deb/cbdb_jammy/debian /gpdb_src/
WORKDIR /gpdb_src
#RUN ./configure --without-perl --without-python --with-libxml --with-gssapi --disable-orca --prefix=/opt/greenplum-db-cb > /dev/null \
# && make -j$(nproc) > /dev/null \
# && make install \
# && echo "/usr/local/lib" >> /etc/ld.so.conf && ldconfig

RUN dpkg-buildpackage -us -uc