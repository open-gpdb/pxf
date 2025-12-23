FROM pxf/singlecluster:3

# copy hadoop files from this image later...

FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

# Install dependencies
# TODO: add pyton3-psutil to greenplum dependencies
RUN apt-get update && \
    apt-get install -y wget lsb-release locales openjdk-11-jre-headless openjdk-8-jre-headless iproute2 sudo python-psutil && \
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

COPY docker/universe/conf/90-gpdb-sysctl.conf /etc/sysctl.d/90-gpdb-sysctl.conf
COPY docker/universe/conf/limits.conf /etc/security/limits.d/limits.conf

# Copy Greenplum:
COPY docker/universe/debs/greenplum-db-6_6.0.0-bionic-dev_amd64.deb /tmp/
RUN apt-get install -y /tmp/greenplum-db-6_6.0.0-bionic-dev_amd64.deb && \
    rm /tmp/greenplum-db-6_6.0.0-bionic-dev_amd64.deb

COPY docker/universe/gpinitsystem_singlenode /tmp/

# Install PXF
COPY docker/universe/debs/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb /tmp/
RUN apt-get install -y /tmp/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb && \
    rm /tmp/greenplum-pxf-6_6.10.1-bionic-dev_amd64.deb

# --------------------------------------------------------------------
# Install automation dependencies
# --------------------------------------------------------------------

# automation has tests with CP1251 encoding
RUN localedef -c -i ru_RU -f CP1251 ru_RU.CP1251

# automation uses golang
RUN wget -O - "https://go.dev/dl/go1.24.5.linux-amd64.tar.gz" | tar -C /usr/local -xz && \
	export PATH="/usr/local/go/bin:$PATH" && \
	go version
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

# automation uses maven and different tools
RUN apt-get install -y maven unzip openssh-server

# configure ssh as described in automation/README.md
RUN ssh-keygen -A && \
    echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config && \
    mkdir -p /etc/ssh/sshd_config.d && \
    touch /etc/ssh/sshd_config.d/pxf-automation.conf && \
    echo "KexAlgorithms +diffie-hellman-group-exchange-sha1,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1" >> /etc/ssh/sshd_config.d/pxf-automation.conf && \
    echo "HostKeyAlgorithms +ssh-rsa,ssh-dss" >> /etc/ssh/sshd_config.d/pxf-automation.conf && \
    echo "PubkeyAcceptedAlgorithms +ssh-rsa,ssh-dss" >> /etc/ssh/sshd_config.d/pxf-automation.conf

# --------------------------------------------------------------------
# Add gpadmin to sudoers and run preparation scripts as gpadmin
# --------------------------------------------------------------------
RUN usermod -a -G sudo gpadmin && \
  echo "gpadmin:cbdb@123" | chpasswd && \
  echo "gpadmin        ALL=(ALL)       NOPASSWD: ALL" >> /etc/sudoers && \
  echo "root           ALL=(ALL)       NOPASSWD: ALL" >> /etc/sudoers

# Base system setup complete, switch to gpadmin to set up everything else
USER gpadmin

# ----------------------------------------------------------------------
# Configure passwordless SSH access for 'gpadmin' user
# ----------------------------------------------------------------------
# The script sets up SSH key-based authentication for the 'gpadmin' user,
# allowing passwordless SSH access. It generates a new SSH key pair if one
# does not already exist, and configures the necessary permissions.
# ----------------------------------------------------------------------
RUN \
  mkdir /home/gpadmin/.ssh && chown -R gpadmin:gpadmin /home/gpadmin/.ssh && \
  ssh-keygen -t rsa -b 4096 -m PEM -C gpadmin -f /home/gpadmin/.ssh/id_rsa -P "" && \
  cat /home/gpadmin/.ssh/id_rsa.pub >> /home/gpadmin/.ssh/authorized_keys && \
  chmod 0600 /home/gpadmin/.ssh/authorized_keys

# --------------------------------------------------------------------
# Setup the Hadoop singlecluster
# --------------------------------------------------------------------
# Setup the Hadoop singlecluster
ENV GPHD_ROOT=/home/gpadmin/workspace/singlecluster
RUN mkdir -p $GPHD_ROOT && chown gpadmin:gpadmin $GPHD_ROOT

ENV HADOOP_ROOT=$GPHD_ROOT/hadoop
ENV HBASE_ROOT=$GPHD_ROOT/hbase
ENV HIVE_ROOT=$GPHD_ROOT/hive
ENV ZOOKEEPER_ROOT=$GPHD_ROOT/zookeeper
ENV PATH=$PATH:$GPHD_ROOT/bin:$HADOOP_ROOT/bin:$HBASE_ROOT/bin:$HIVE_ROOT/bin:$ZOOKEEPER_ROOT/bin

COPY --from=pxf/singlecluster:3 $GPHD_ROOT $GPHD_ROOT
RUN sudo chown -R gpadmin:gpadmin $GPHD_ROOT

# --------------------------------------------------------------------
# Run tests
# --------------------------------------------------------------------
ENV GPHOME=/opt/greenplum-db-6/
ENV PXF_HOME=/opt/greenplum-pxf-6/
ENV PXF_BASE=/home/gpadmin/pxf

EXPOSE 5432 22

# Copy automation directory into container
RUN mkdir -p /home/gpadmin/workspace/pxf
COPY . /home/gpadmin/workspace/pxf/automation
RUN sudo chown -R gpadmin:gpadmin /home/gpadmin/workspace/pxf/automation
RUN sudo chmod -R 755 /home/gpadmin/workspace/pxf/automation

COPY docker/universe/entrypoint.sh /entrypoint.sh
RUN sudo chmod +x /entrypoint.sh
COPY docker/universe/run_tests.sh /run_tests.sh
RUN sudo chmod +x /run_tests.sh