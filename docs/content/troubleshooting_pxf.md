---
title: Troubleshooting
---

<!--
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-->


## <a id="pxf-errors"></a>PXF Errors
The following table describes some errors you may encounter while using PXF:

| Error Message                 | Discussion                     |
|-------------------------------|---------------------------------|
| Protocol "pxf" does not exist | **Cause**: The `pxf` extension was not registered.<br>**Solution**: Create (enable) the PXF extension for the database as described in the PXF [Enable Procedure](using_pxf.html#enable-pxf-ext).|
| Invalid URI pxf://\<path-to-data\>: missing options section | **Cause**: The `LOCATION` URI does not include the profile or other required options.<br>**Solution**: Provide the profile and required options in the URI when you submit the `CREATE EXTERNAL TABLE` command. |
| PXF server error : Input path does not exist: hdfs://\<namenode\>:8020/\<path-to-file\> | **Cause**: The HDFS file that you specified in \<path-to-file\> does not exist. <br>**Solution**: Provide the path to an existing HDFS file. |
| PXF server error : NoSuchObjectException(message:\<schema\>.\<hivetable\> table not found) | **Cause**: The Hive table that you specified with \<schema\>.\<hivetable\> does not exist. <br>**Solution**: Provide the name of an existing Hive table. |
| PXF server error : Failed connect to localhost:5888; Connection refused (\<segment-id\> slice\<N\> \<segment-host\>:\<port\> pid=\<process-id\>)<br> ... |**Cause**: The PXF Service is not running on \<segment-host\>.<br>**Solution**: Restart PXF on \<segment-host\>. |
| PXF server error: Permission denied: user=\<user\>, access=READ, inode=&quot;\<filepath\>&quot;:-rw------- | **Cause**: The Greenplum Database user that ran the PXF operation does not have permission to access the underlying Hadoop service (HDFS or Hive). See [Configuring the Hadoop User, User Impersonation, and Proxying](pxfuserimpers.html). |
| PXF server error: PXF service could not be reached. PXF is not running in the tomcat container | **Cause**: The `pxf` extension was updated to a new version but the PXF server has not been updated to a compatible version. <br>**Solution**: Ensure that the PXF server has been updated and restarted on all hosts. |
| ERROR: could not load library "/usr/local/greenplum-db-x.x.x/lib/postgresql/pxf.so" | **Cause**: Some steps have not been completed after a Greenplum Database upgrade or migration, such as `pxf cluster register`. <br>**Solution**: Make sure you follow the steps outlined for [PXF Upgrade and Migration](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/pxf-pxf_upgrade_migration.html. | 

Most PXF error messages include a `HINT` that you can use to resolve the error, or to collect more information to identify the error.

## <a id="pxf-loggin"></a>PXF Logging

Refer to the [Logging](cfg_logging.html) topic for more information about logging levels, configuration, and the `pxf-app.out` and `pxf-service.log` log files.


## <a id="pxf-timezonecfg"></a>Addressing PXF JDBC Connector Time Zone Errors

You use the PXF JDBC connector to access data stored in an external SQL database. Depending upon the JDBC driver, the driver may return an error if there is a mismatch between the default time zone set for the PXF Service and the time zone set for the external SQL database.

For example, if you use the PXF JDBC connector to access an Oracle database with a conflicting time zone, PXF logs an error similar to the following:

``` pre
java.io.IOException: ORA-00604: error occurred at recursive SQL level 1
ORA-01882: timezone region not found
```

Should you encounter this error, you can set default time zone option(s) for the PXF Service in the `$PXF_BASE/conf/pxf-env.sh` configuration file, `PXF_JVM_OPTS` property setting. For example, to set the time zone:

``` pre
export PXF_JVM_OPTS="<current_settings> -Duser.timezone=America/Chicago"
```

You can use the `PXF_JVM_OPTS` property to set other Java options as well.

As described in previous sections, you must synchronize the updated PXF configuration to the Greenplum Database cluster and restart the PXF Service on each host.


## <a id="pxf-tblpart"></a>About PXF External Table Child Partitions

Greenplum Database supports partitioned tables, and permits exchanging a leaf child partition with a PXF external table.

When you read from a partitioned Greenplum table where one or more partitions is a PXF external table and there is no data backing the external table path, PXF returns an error and the query fails. This default PXF behavior is not optimal in the partitioned table case; an empty child partition is valid and should not cause a query on the parent table to fail.

The `IGNORE_MISSING_PATH` PXF custom option is a boolean that specifies the action to take when the external table path is missing or invalid. The default value is `false`, PXF returns an error when it encounters a missing path. If the external table is a child partition of a Greenplum table, you want PXF to ignore a missing path error, so set this option to `true`.

For example, PXF ignores missing path errors generated from the following external table:

``` sql
CREATE EXTERNAL TABLE ext_part_87 (id int, some_date date)
  LOCATION ('pxf://bucket/path/?PROFILE=s3:parquet&SERVER=s3&IGNORE_MISSING_PATH=true')
FORMAT 'CUSTOM' (formatter = 'pxfwritable_import');
```

The `IGNORE_MISSING_PATH` custom option applies only to file-based profiles, including `*:text`, `*:csv`, `*:fixedwidth`, `*:parquet`, `*:avro`, `*:json`, `*:AvroSequenceFile`, and `*:SequenceFile`. This option is *not available* when the external table specifies the `hbase`, `hive[:*]`, or `jdbc` profiles, or when reading from S3 using S3-Select.


## <a id="hive-metastore"></a>Addressing Hive MetaStore Connection Errors

The PXF Hive connector uses the Hive MetaStore to determine the HDFS locations of Hive tables. Starting in PXF version 6.2.1, PXF retries a failed connection to the Hive MetaStore a single time. If you encounter one of the following error messages or exceptions when accessing Hive via a PXF external table, consider increasing the retry count:

- `Failed to connect to the MetaStore Server.`
- `Could not connect to meta store ...`
- `org.apache.thrift.transport.TTransportException: null`

PXF uses the `hive-site.xml` `hive.metastore.failure.retries` property setting to identify the maximum number of times it will retry a failed connection to the Hive MetaStore. The `hive-site.xml` file resides in the configuration directory of the PXF server that you use to access Hive.

Perform the following procedure to configure the number of Hive MetaStore connection retries that PXF will attempt; you may be required to add the `hive.metastore.failure.retries` property to the `hive-site.xml` file:

1. Log in to the Greenplum Database coordinator host.

1. Identify the name of your Hive PXF server.

1. Open the `$PXF_BASE/servers/<hive-server-name>/hive-site.xml` file in the editor of your choice, add the `hive.metastore.failure.retries` property if it does not already exist in the file, and set the value. For example, to configure 5 retries: 

    ``` xml
    <property>
        <name>hive.metastore.failure.retries</name>
        <value>5</value>
    </property>
    ```

1. Save the file and exit the editor.

1. Synchronize the PXF configuration to all hosts in your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

1. Re-run the failing SQL external table command.

## <a id="lzo"></a>Addressing a Missing Compression Codec Error

By default, PXF does not bundle the LZO compression library. If the Hadoop cluster is configured to use LZO compression, PXF returns the error message `Compression codec com.hadoop.compression.lzo.LzoCodec not found` on first access to Hadoop. To remedy the situation, you must register the LZO compression library with PXF as described below (for more information, refer to [Registering a JAR Dependency](reg_jar_depend.html#reg_jar)):

1. Locate the LZO library in the Hadoop installation directory on the Hadoop NameNode. For example, the file system location of the library may be `/usr/lib/hadoop-lzo/lib/hadoop-lzo.jar`.

1. Log in to the Greenplum Database coordinator host.

1. Copy `hadoop-lzo.jar` from the Hadoop NameNode to the PXF configuration directory on the Greenplum Database coordinator host. For example, if `$PXF_BASE` is `/usr/local/pxf-gp6`:

    ``` shell
    gpadmin@coordinator$ scp <hadoop-user>@<namenode-host>:/usr/lib/hadoop-lzo/lib/hadoop-lzo.jar /usr/local/pxf-gp6/lib/
    ```

1. Synchronize the PXF configuration and restart PXF:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    gpadmin@coordinator$ pxf cluster restart
    ```

1. Re-run the query.

## <a id="snappy-init"></a>Addressing a Snappy Compression Initialization Error

Snappy compression requires an executable temporary directory in which to load its native library. If you are using PXF to read or write a snappy-compressed Avro, ORC, or Parquet file and encounter the error `java.lang.NoClassDefFoundError: Could not initialize class org.xerial.snappy.Snappy`, the temporary directory used by Snappy (default is `/tmp`) may not be executable.

To remedy this situation, specify an executable directory for the Snappy `tempdir`. This procedure involves stopping PXF, updating PXF configuration, synchronizing the configuration change, and then restarting PXF as follows:

1. Determine if the `/tmp` directory is executable:

    ``` shell
    $ mount | grep '/tmp'
    tmpfs on /tmp type tmpfs (rw,nosuid,nodev,noexec,seclabel)
    ```

    A `noexec` option in the `mount` output indicates that the directory is not executable.

    Perform this check on each Greenplum Database host.

1. If the `mount` command output for `/tmp` does not include `noexec`, the directory is executable. Exit this procedure, the workaround will not address your issue.

    If the `mount` command output for `/tmp` includes `noexec`, continue.

1. Log in to the Greenplum Database coordinator host.

1. Stop PXF on the Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster stop
    ```

1.  Locate the `pxf-env.sh` file in your PXF installation. If you did not relocate `$PXF_BASE`, the file is located here:

    ``` pre
    /usr/local/pxf-gp6/conf/pxf-env.sh
    ```

1.  Open `pxf-env.sh` in the editor of your choice, locate the line where `PXF_JVM_OPTS` is set, uncomment the line if it is not already uncommented, and add `-Dorg.xerial.snappy.tempdir=${PXF_BASE}/run` to the setting. For example:

    ``` shell
    # Memory
    export PXF_JVM_OPTS="-Xmx2g -Xms1g -Dorg.xerial.snappy.tempdir=${PXF_BASE}/run"
    ```

    This option sets the Snappy temporary directory to `${PXF_BASE}/run`, an executable directory accessible by PXF.

1.  Synchronize the PXF configuration and then restart PXF:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    gpadmin@coordinator$ pxf cluster start
    ```

## <a id="hiveorcnulls"></a>Reading from a Hive table STORED AS ORC Returns NULLs

If you are using PXF to read from a Hive table `STORED AS ORC` and one or more columns that have values are returned as NULLs, there may be a case sensitivity issue between the column names specified in the Hive table definition and those specified in the ORC embedded schema definition. This might happen if the table has been created and populated by another system such as Spark.

The workaround described in this section applies when all of the following hold true:

- The Greenplum Database PXF external table that you created specifies the `hive:orc` profile.
- The Greenplum Database PXF external table that you created specifies the `VECTORIZE=false` (the default) setting.
- There is a case mis-match between the column names specified in the Hive table schema and the column names specified in the ORC embedded schema.
- You confirm that the field names in the ORC embedded schema are not all in lowercase by performing the following tasks:
    1. Run `DESC FORMATTED <table-name>` in the `hive` subsystem and note the returned `location`; for example, `location:hdfs://namenode/hive/warehouse/<table-name>`.
    1. List the ORC files comprising the table by running the following command:

        ``` shell
        $ hdfs dfs -ls <location>
        ```
    1. Dump each ORC file with the following command. For example, if the first step returned `hdfs://namenode/hive/warehouse/hive_orc_tbl1, run:`

        ``` shell
        $ hive --orcfiledump /hive/warehouse/hive_orc_tbl1/<orc-file> > dump.out
        ```
    1. Examine the output, specifically the value of `Type` (sample output: `Type: struct<COL0:int,COL1:string>`). If the field names are not all lowercase, continue with the workaround below.

*To remedy this situation, perform the following procedure*:

1. Log in to the Greenplum Database coordinator host.

1. Identify the name of your Hadoop PXF server configuration.

1. Locate the `hive-site.xml` configuration file in the server configuration directory. For example, if `$PXF_BASE` is `/usr/local/pxf-gp6` and the server name is `<server_name>`, the file is located here:

    ``` pre
    /usr/local/pxf-gp6/servers/<server_name>/hive-site.xml
    ```

1. Add or update the following property definition in the `hive-site.xml` file, and then save and exit the editor:

    ``` xml
    <property>
        <name>orc.schema.evolution.case.sensitive</name>
        <value>false</value>
        <description>A boolean flag to determine if the comparision of field names in schema evolution is case sensitive.</description>
    </property>
    ```

1. Synchronize the PXF server configuration to your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ````

1. Try the query again.

