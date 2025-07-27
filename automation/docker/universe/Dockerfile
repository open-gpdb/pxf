FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

# Install dependencies
# TODO: add pyton3-psutil to greenplum dependencies
RUN apt-get update && \
    apt-get install -y wget lsb-release locales openjdk-11-jre-headless iproute2 sudo python-psutil && \
    locale-gen en_US.UTF-8 && \
    locale-gen ru_RU.CP1251 && \
    locale-gen ru_RU.UTF-8 && \
    update-locale LANG=en_US.UTF-8

# --------------------------------------------------------------------
# Install Greenplum and PXF
# --------------------------------------------------------------------

ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Copy Greenplum:
COPY debs/greenplum-db-6_6.0.0-bionic-dev_amd64.deb /tmp/
RUN apt-get install -y /tmp/greenplum-db-6_6.0.0-bionic-dev_amd64.deb && \
    rm /tmp/greenplum-db-6_6.0.0-bionic-dev_amd64.deb
# Install PXF
COPY debs/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb /tmp/
RUN apt-get install -y /tmp/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb && \
    rm /tmp/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb

# --------------------------------------------------------------------
# Install automation
# --------------------------------------------------------------------

# automation has tests with CP1251 encoding
RUN localedef -c -i ru_RU -f CP1251 ru_RU.CP1251

# automation uses golang
RUN wget -O - "https://go.dev/dl/go1.24.5.linux-amd64.tar.gz" | tar -C /usr/local -xz && \
	export PATH="/usr/local/go/bin:$PATH" && \
	go version
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

# automation uses maven and different tools
RUN apt-get install -y maven unzip

# --------------------------------------------------------------------
# Add gpadmin to sudoers and run preparation scripts as gpadmin
# --------------------------------------------------------------------
RUN usermod -a -G sudo gpadmin && \
  echo "gpadmin:cbdb@123" | chpasswd && \
  echo "gpadmin        ALL=(ALL)       NOPASSWD: ALL" >> /etc/sudoers && \
  echo "root           ALL=(ALL)       NOPASSWD: ALL" >> /etc/sudoers \

USER gpadmin

# --------------------------------------------------------------------
# Run tests
# --------------------------------------------------------------------
ENV PXF_HOME=/opt/greenplum-pxf-6/
ENV PXF_BASE=/home/gpadmin/pxf
RUN mkdir -p ${PXF_BASE}/servers/default/
RUN cp ${PXF_HOME}/templates/hbase-site.xml \
    ${PXF_HOME}/templates/hdfs-site.xml \
    ${PXF_HOME}/templates/hive-site.xml \
    ${PXF_HOME}/templates/core-site.xml \
    ${PXF_HOME}/templates/mapred-site.xml \
    ${PXF_HOME}/templates/pxf-site.xml \
    ${PXF_HOME}/templates/jdbc-site.xml \
    ${PXF_BASE}/servers/default/

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]