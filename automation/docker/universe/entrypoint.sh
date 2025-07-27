#!/bin/bash
set -e

# ----------------------------------------------------------------------
# Start SSH daemon and setup for SSH access
# ----------------------------------------------------------------------
# The SSH daemon is started to allow remote access to the container via
# SSH. This is useful for development and debugging purposes. If the SSH
# daemon fails to start, the script exits with an error.
# ----------------------------------------------------------------------
if [ ! -d /var/run/sshd ]; then
   sudo mkdir /var/run/sshd
   sudo chmod 0755 /var/run/sshd
fi
if ! sudo /usr/sbin/sshd; then
    echo "Failed to start SSH daemon"
    exit 1
fi

# ----------------------------------------------------------------------
# Remove /run/nologin to allow logins for all users via SSH
# ----------------------------------------------------------------------
sudo rm -rf /run/nologin

# ----------------------------------------------------------------------
# Prepare files for gpinitsystem
# ----------------------------------------------------------------------
sudo mkdir -p /data0/database/master /data0/database/primary /data0/database/mirror
sudo chown -R gpadmin:gpadmin /data0

echo "mdw" | sudo tee -a /tmp/gpdb-hosts

sudo chown -R gpadmin:gpadmin /tmp/gpinitsystem_singlenode /tmp/gpdb-hosts
echo "export COORDINATOR_DATA_DIRECTORY=/data0/database/master/gpseg-1" | sudo tee -a /etc/profile
echo "export MASTER_DATA_DIRECTORY=/data0/database/master/gpseg-1"      | sudo tee -a /etc/profile
echo "source /opt/greenplum-db-6/greenplum_path.sh"                     | sudo tee -a /etc/profile

# ----------------------------------------------------------------------
# Configure /home/gpadmin
# ----------------------------------------------------------------------
mkdir -p /home/gpadmin/.ssh/
ssh-keyscan -t rsa mdw > /home/gpadmin/.ssh/known_hosts
chown -R gpadmin:gpadmin /home/gpadmin/.ssh/

echo "export COORDINATOR_DATA_DIRECTORY=/data0/database/master/gpseg-1" >> /home/gpadmin/.bashrc
echo "export MASTER_DATA_DIRECTORY=/data0/database/master/gpseg-1"      >> /home/gpadmin/.bashrc
echo "source /opt/greenplum-db-6/greenplum_path.sh"                     >> /home/gpadmin/.bashrc

# ----------------------------------------------------------------------
# Run gpinitsystem
# ----------------------------------------------------------------------
# Source Cloudberry environment variables
source /opt/greenplum-db-6/greenplum_path.sh
export COORDINATOR_DATA_DIRECTORY=/data0/database/master/gpseg-1
export MASTER_DATA_DIRECTORY=/data0/database/master/gpseg-1

export USER=gpadmin

# Initialize single node Cloudberry cluster
gpinitsystem -a \
             -c /tmp/gpinitsystem_singlenode \
             -h /tmp/gpdb-hosts \
             --max_connections=100 || echo "gpinitsystem finished with exit code $?"

## Allow any host access the Cloudberry Cluster
echo 'host all all 0.0.0.0/0 trust' >> /data0/database/master/gpseg-1/pg_hba.conf

# Configure PostgreSQL to listen on all interfaces
echo "listen_addresses = '*'" >> /data0/database/master/gpseg-1/postgresql.conf
echo "port = 5432" >> /data0/database/master/gpseg-1/postgresql.conf

gpstop -u && echo "pg_hba.conf has been reloaded"

psql -d template1 \
     -c "ALTER USER gpadmin PASSWORD 'cbdb@123'"

## Set gpadmin password, display version and cluster configuration
psql -P pager=off -d template1 -c "SELECT VERSION()"
psql -P pager=off -d template1 -c "SELECT * FROM gp_segment_configuration ORDER BY dbid"
psql -P pager=off -d template1 -c "SHOW optimizer"


# ----------------------------------------------------------------------
# Prepare PXF
# ----------------------------------------------------------------------
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
export PATH="$PXF_HOME/bin:$PATH"
export PXF_JVM_OPTS="-Xmx512m -Xms256m"
export PXF_HOST=localhost # 0.0.0.0  # listen on all interfaces

# Prepare a new $PXF_BASE directory on each Greenplum Database host.
# - create directory structure in $PXF_BASE
# - copy configuration files from $PXF_HOME/conf to $PXF_BASE/conf
/opt/greenplum-pxf-6/bin/pxf cluster prepare

# Use Java 11:
echo "JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64" >> $PXF_BASE/conf/pxf-env.sh
# Configure PXF to listen on all interfaces
sed -i 's/# server.address=localhost/server.address=0.0.0.0/' /home/gpadmin/pxf/conf/pxf-application.properties
# add property to allow dynamic test: profiles that are used when testing against FDW
echo -e "\npxf.profile.dynamic.regex=test:.*" >> $PXF_BASE/conf/pxf-application.properties
# set up pxf configs from templates
cp -v $PXF_HOME/templates/{hdfs,mapred,yarn,core,hbase,hive}-site.xml $PXF_BASE/servers/default

# Register PXF extension in Greenplum
# - Copy the PXF extension control file from the PXF installation on each host to the Greenplum installation on the host
/opt/greenplum-pxf-6/bin/pxf cluster register
# Start PXF
/opt/greenplum-pxf-6/bin/pxf cluster start

# --------------------------------------------------------------------
# Run tests
# --------------------------------------------------------------------
cd /home/gpadmin/workspace/pxf/automation
# make without arguments runs all tests
make

# Keep container running
#tail -f /dev/null