---
title: Configuring Hadoop Connectors (Optional)
---

PXF is compatible with Cloudera, Hortonworks Data Platform, and generic Apache Hadoop distributions. This topic describes how to configure the PXF Hadoop, Hive, and HBase connectors.

*If you do not want to use the Hadoop-related PXF connectors, then you do not need to perform this procedure.*

## <a id="prereq"></a>Prerequisites

Configuring PXF Hadoop connectors involves copying configuration files from your Hadoop cluster to the Greenplum Database coordinator host. Before you configure the PXF Hadoop connectors, ensure that you can copy files from hosts in your Hadoop cluster to the Greenplum Database coordinator.


## <a id="client-pxf-config-steps"></a>Procedure

Perform the following procedure to configure the desired PXF Hadoop-related connectors on the Greenplum Database coordinator host. After you configure the connectors, you will use the `pxf cluster sync` command to copy the PXF configuration to the Greenplum Database cluster.

In this procedure, you use the `default`, or create a new PXF server configuration. You copy Hadoop configuration files to the server configuration directory on the Greenplum Database coordinator host. You identify Kerberos and user impersonation settings required for access, if applicable. You then synchronize the PXF configuration on the coordinator host to the standby coordinator host and segment hosts.

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Identify the name of your PXF Hadoop server configuration.

3. If you are not using the `default` PXF server, create the `$PXF_BASE/servers/<server_name>` directory. For example, use the following command to create a Hadoop server configuration named `hdp3`:

    ``` shell
    gpadmin@coordinator$ mkdir $PXF_BASE/servers/hdp3
    ````

4. Change to the server directory. For example:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/default
    ```

    Or,

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

2. PXF requires information from `core-site.xml` and other Hadoop configuration files. Copy the `core-site.xml`, `hdfs-site.xml`, `mapred-site.xml`, and `yarn-site.xml` Hadoop configuration files from your Hadoop cluster NameNode host to the current host using your tool of choice. Your file paths may differ based on the Hadoop distribution in use. For example, these commands use `scp` to copy the files:

    ``` shell
    gpadmin@coordinator$ scp hdfsuser@namenode:/etc/hadoop/conf/core-site.xml .
    gpadmin@coordinator$ scp hdfsuser@namenode:/etc/hadoop/conf/hdfs-site.xml .
    gpadmin@coordinator$ scp hdfsuser@namenode:/etc/hadoop/conf/mapred-site.xml .
    gpadmin@coordinator$ scp hdfsuser@namenode:/etc/hadoop/conf/yarn-site.xml .
    ```
        
3. If you plan to use the PXF Hive connector to access Hive table data, similarly copy the Hive configuration to the Greenplum Database coordinator host. For example:

    ``` shell
    gpadmin@coordinator$ scp hiveuser@hivehost:/etc/hive/conf/hive-site.xml .
    ```

4. If you plan to use the PXF HBase connector to access HBase table data, similarly copy the HBase configuration to the Greenplum Database coordinator host. For example:
    
    ``` shell
    gpadmin@coordinator$ scp hbaseuser@hbasehost:/etc/hbase/conf/hbase-site.xml .
    ```

5. Synchronize the PXF configuration to the Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

4. PXF accesses Hadoop services on behalf of Greenplum Database end users. By default, PXF tries to access HDFS, Hive, and HBase using the identity of the Greenplum Database user account that logs into Greenplum Database. In order to support this functionality, you must configure proxy settings for Hadoop, as well as for Hive and HBase if you intend to use those PXF connectors. Follow procedures in [Configuring User Impersonation and Proxying](pxfuserimpers.html) to configure user impersonation and proxying for Hadoop services, or to turn off PXF user impersonation.

5. Grant read permission to the HDFS files and directories that will be accessed as external tables in Greenplum Database. If user impersonation is enabled (the default), you must grant this permission to each Greenplum Database user/role name that will use external tables that reference the HDFS files. If user impersonation is not enabled, you must grant this permission to the `gpadmin` user.

6. If your Hadoop cluster is secured with Kerberos, you must configure PXF and generate Kerberos principals and keytabs for each Greenplum Database host as described in [Configuring PXF for Secure HDFS](pxf_kerbhdfs.html).


## <a id="client-cfg-update"></a>About Updating the Hadoop Configuration

If you update your Hadoop, Hive, or HBase configuration while the PXF Service is running, you must copy the updated configuration to the `$PXF_BASE/servers/<server_name>` directory and re-sync the PXF configuration to your Greenplum Database cluster. For example:

``` shell
gpadmin@coordinator$ cd $PXF_BASE/servers/<server_name>
gpadmin@coordinator$ scp hiveuser@hivehost:/etc/hive/conf/hive-site.xml .
gpadmin@coordinator$ pxf cluster sync
```

