---
title: Configuring PXF Servers
---

This topic provides an overview of PXF server configuration. To configure a server, refer to the topic specific to the connector that you want to configure.

You read from or write data to an external data store via a PXF connector. To access an external data store, you must provide the server location. You may also be required to provide client access credentials and other external data store-specific properties. PXF simplifies configuring access to external data stores by:

- Supporting file-based connector and user configuration
- Providing connector-specific template configuration files

A PXF *Server* definition is simply a named configuration that provides access to a specific external data store. A PXF server name is the name of a directory residing in `$PXF_BASE/servers/`. The information that you provide in a server configuration is connector-specific. For example, a PXF JDBC Connector server definition may include settings for the JDBC driver class name, URL, username, and password. You can also configure connection-specific and session-specific properties in a JDBC server definition.

PXF provides a server template file for each connector; this template identifies the typical set of properties that you must configure to use the connector.

You will configure a server definition for each external data store that Greenplum Database users need to access. For example, if you require access to two Hadoop clusters, you will create a PXF Hadoop server configuration for each cluster. If you require access to an Oracle and a MySQL database, you will create one or more PXF JDBC server configurations for each database.

A server configuration may include default settings for user access credentials and other properties for the external data store. You can allow Greenplum Database users to access the external data store using the default settings, or you can configure access and other properties on a per-user basis. This allows you to configure different Greenplum Database users with different external data store access credentials in a single PXF server definition.


## <a id="templates"></a>About Server Template Files

The configuration information for a PXF server resides in one or more `<connector>-site.xml` files in `$PXF_BASE/servers/<server_name>/`.

PXF provides a template configuration file for each connector.  These server template configuration files are located in the `<PXF_INSTALL_DIR>/templates/` directory after you install PXF:

```
gpadmin@coordinator$ ls <PXF_INSTALL_DIR>/templates
abfss-site.xml   hbase-site.xml  jdbc-site.xml    pxf-site.xml    yarn-site.xml
core-site.xml  hdfs-site.xml   mapred-site.xml  s3-site.xml
gs-site.xml    hive-site.xml   minio-site.xml   wasbs-site.xml
```

For example, the contents of the `s3-site.xml` template file follow:

``` pre
<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <property>
        <name>fs.s3a.access.key</name>
        <value>YOUR_AWS_ACCESS_KEY_ID</value>
    </property>
    <property>
        <name>fs.s3a.secret.key</name>
        <value>YOUR_AWS_SECRET_ACCESS_KEY</value>
    </property>
    <property>
        <name>fs.s3a.fast.upload</name>
        <value>true</value>
    </property>
</configuration>
```

<div class="note"><b>Note:</b> You specify credentials to PXF in clear text in configuration files.</div>

**Note**: The template files for the Hadoop connectors are not intended to be modified and used for configuration, as they only provide an example of the information needed. Instead of modifying the Hadoop templates, you will copy several Hadoop `*-site.xml` files from the Hadoop cluster to your PXF Hadoop server configuration.


## <a id="default"></a>About the Default Server

PXF defines a special server named `default`. The PXF installation creates a `$PXF_BASE/servers/default/` directory. This directory, initially empty, identifies the default PXF server configuration. You can configure and assign the default PXF server to any external data source. For example, you can assign the PXF default server to a Hadoop cluster, or to a MySQL database that your users frequently access.

PXF automatically uses the `default` server configuration if you omit the `SERVER=<server_name>` setting in the `CREATE EXTERNAL TABLE` command `LOCATION` clause.


## <a id="cfgproc"></a>Configuring a Server

When you configure a PXF connector to an external data store, you add a named PXF server configuration for the connector. Among the tasks that you perform, you may:

1. Determine if you are configuring the `default` PXF server, or choose a new name for the server configuration.
2. Create the directory `$PXF_BASE/servers/<server_name>`.
3. Copy template or other configuration files to the new server directory.
4. Fill in appropriate default values for the properties in the template file.
5. Add any additional configuration properties and values required for your environment.
6. Configure one or more users for the server configuration as described in [About Configuring a PXF User](#usercfg).
7. Synchronize the server and user configuration to the Greenplum Database cluster.

**Note**: You must re-sync the PXF configuration to the Greenplum Database cluster after you add or update PXF server configuration.

After you configure a PXF server, you publish the server name to Greenplum Database users who need access to the data store. A user need only provide the server name when they create an external table that accesses the external data store. PXF obtains the external data source location and access credentials from server and user configuration files residing in the server configuration directory identified by the server name.

To configure a PXF server, refer to the connector configuration topic:

- To configure a PXF server for Hadoop, refer to [Configuring PXF Hadoop Connectors ](client_instcfg.html).
- To configure a PXF server for an object store, refer to [Configuring Connectors to MinIO, AWS S3, and Dell ECS Object Stores](s3_objstore_cfg.html) and [Configuring Connectors to Azure and Google Cloud Storage Object Stores](objstore_cfg.html).
- To configure a PXF JDBC server, refer to [Configuring the JDBC Connector ](jdbc_cfg.html).
- [Configuring a PXF Network File System Server](nfs_pxf.html#ex_fscfg) describes the process of configuring a PXF server for network file system access.

## <a id="pxf-site"></a>About the pxf-site.xml Configuration File

PXF includes a template file named `pxf-site.xml` for PXF-specific configuration parameters. You can use the `pxf-site.xml` template file to configure:

- Kerberos and/or user impersonation settings for server configurations
- a base directory for file access
- the action of PXF when it detects an overflow condition while writing numeric ORC or Parquet data

<div class="note"><b>Note:</b> The Kerberos and user impersonation settings in this file may apply only to Hadoop and JDBC server configurations; they do not apply to file system or object store server configurations.</div>

You configure properties in the `pxf-site.xml` file for a PXF server when one or more of the following conditions hold:

- The remote Hadoop system utilizes Kerberos authentication.
- You want to activate/deactivate user impersonation on the remote Hadoop or external database system.
- You want to activate/deactivate Kerberos constrained delegation for a Hadoop PXF server.
- You will access a network file system with the server configuration.
- You will access a remote Hadoop or object store file system with the server configuration, and you want to allow a user to access only a specific directory and subdirectories.

`pxf-site.xml` includes the following properties:

| Property       | Description                                | Default Value |
|----------------|--------------------------------------------|---------------|
| pxf.service.kerberos.principal | The Kerberos principal name. | gpadmin/\_HOST@EXAMPLE.COM |
| pxf.service.kerberos.keytab | The file system path to the Kerberos keytab file. | $PXF_BASE/keytabs/pxf.service.keytab |
| pxf.service.kerberos.constrained-delegation | Activates/deactivates [Kerberos constrained delegation](pxf_kerbhdfs.html#kcd). **Note:** This property is applicable only to Hadoop PXF server configurations, it is not applicable to JDBC PXF servers. | false |
| pxf.service.kerberos.ticket-renew-window| The minimum elapsed lifespan (as a percentage) after which PXF attempts to renew/refresh a Kerberos ticket. Value range is from 0 (PXF generates a new ticket for all requests) to 1 (PXF renews after full ticket lifespan). | 0.8 (80%) |
| pxf.service.user.impersonation | Activates/deactivates user impersonation when connecting to the remote system. | If the `pxf.service.user.impersonation` property is missing from `pxf-site.xml`, the default is `true` (activated) for PXF Hadoop servers and `false` (deactivated) for JDBC servers. |
| pxf.service.user.name | The login user for the remote system. | This property is commented out by default. When the property is unset, the default value is the operating system user that starts the pxf process, typically `gpadmin`. When the property is set, the default value depends on the user impersonation setting and, if you are accessing Hadoop, whether or not you are accessing a Kerberos-secured cluster; see the [Use Cases and Configuration Scenarios](pxfuserimpers.html#pxf_cfg_scenarios) section in the *Configuring the Hadoop User, User Impersonation, and Proxying* topic. |
| pxf.fs.basePath | Identifies the base path or share point on the remote file system. This property is applicable when the server configuration is used with a profile that accesses a file. | None; this property is commented out by default. |
| pxf.ppd.hive<sup>1</sup> | Specifies whether or not predicate pushdown is enabled for queries on external tables that specify the `hive`, `hive:rc`, or `hive:orc` profiles. | True; predicate pushdown is enabled. |
| pxf.sasl.connection.retries | Specifies the maximum number of times that PXF retries a SASL connection request after a refused connection returns a `GSS initiate failed` error. | 5 |
| pxf.orc.write.decimal.overflow | Specifies how PXF handles numeric data that exceeds the maximum precision of 38 and [overflows](hdfs_orc.html#overflow) when writing to an ORC file. Valid values are: round, error, or ignore | round |
| pxf.parquet.write.decimal.overflow | Specifies how PXF handles numeric data that exceeds the maximum precision of 38 and [overflows](hdfs_parquet.html#overflow) when writing to a Parquet file. Valid values are: round, error, or ignore | round |

</br><sup>1</sup>&nbsp;Should you need to, you can override this setting on a per-table basis by specifying the `&PPD=<boolean>` option in the `LOCATION` clause when you create the external table.

Refer to [Configuring PXF Hadoop Connectors ](client_instcfg.html) and [Configuring the JDBC Connector ](jdbc_cfg.html) for information about relevant `pxf-site.xml` property settings for Hadoop and JDBC server configurations, respectively. See [Configuring a PXF Network File System Server](nfs_pxf.html#ex_fscfg) for information about relevant `pxf-site.xml` property settings when you configure a PXF server to access a network file system.

### <a id="pxf-fs-basepath"></a>About the pxf.fs.basePath Property

You can use the `pxf.fs.basePath` property to restrict a user's access to files in a specific remote directory. When set, this property applies to any profile that accesses a file, including `*:text`, `*:parquet`, `*:json`, etc.

When you configure the `pxf.fs.basePath` property for a server, PXF considers the file path specified in the `CREATE EXTERNAL TABLE` `LOCATION` clause to be relative to this base path setting, and constructs the remote path accordingly.

<div class="note info"><b>Note:</b> You must set <code>pxf.fs.basePath</code> when you configure a PXF server for access to a network file system with a <code>file:*</code> profile. This property is optional for a PXF server that accesses a file in Hadoop or in an object store.</div>


## <a id="usercfg"></a>Configuring a PXF User

You can configure access to an external data store on a per-server, per-Greenplum-user basis.

<div class="note info"><b>Note:</b> PXF per-server, per-user configuration provides the most benefit for JDBC servers.</div>

You configure external data store user access credentials and properties for a specific Greenplum Database user by providing a `<greenplum_user_name>-user.xml` user configuration file in the PXF server configuration directory, `$PXF_BASE/servers/<server_name>/`. For example, you specify the properties for the Greenplum Database user named `bill` in the file `$PXF_BASE/servers/<server_name>/bill-user.xml`. You can configure zero, one, or more users in a PXF server configuration.


The properties that you specify in a user configuration file are connector-specific. You can specify any configuration property supported by the PXF connector server in a `<greenplum_user_name>-user.xml` configuration file.

For example, suppose you have configured access to a PostgreSQL database in the PXF JDBC server configuration named `pgsrv1`. To allow the Greenplum Database user named `bill` to access this database as the PostgreSQL user named `pguser1`, password `changeme`, you create the user configuration file `$PXF_BASE/servers/pgsrv1/bill-user.xml` with the following properties:

``` xml
<configuration>
    <property>
        <name>jdbc.user</name>
        <value>pguser1</value>
    </property>
    <property>
        <name>jdbc.password</name>
        <value>changeme</value>
    </property>
</configuration>
```

If you want to configure a specific search path and a larger read fetch size for `bill`, you would also add the following properties to the `bill-user.xml` user configuration file:

``` xml
    <property>
        <name>jdbc.session.property.search_path</name>
        <value>bill_schema</value>
    </property>
    <property>
        <name>jdbc.statement.fetchSize</name>
        <value>2000</value>
    </property>
```

### <a id="cfgproc_user"></a>Procedure

For each PXF user that you want to configure, you will:

1. Identify the name of the Greenplum Database user.
2. Identify the PXF server definition for which you want to configure user access.
3. Identify the name and value of each property that you want to configure for the user.
4. Create/edit the file `$PXF_BASE/servers/<server_name>/<greenplum_user_name>-user.xml`, and add the outer configuration block:

    ``` xml
    <configuration>
    </configuration>
    ```
5. Add each property/value pair that you identified in Step 3 within the configuration block in the `<greenplum_user_name>-user.xml` file.
6. If you are adding the PXF user configuration to previously configured  PXF server definition, synchronize the user configuration to the Greenplum Database cluster.


## <a id="override"></a>About Configuration Property Precedence

A PXF server configuration may include default settings for user access credentials and other properties for accessing an external data store. Some PXF connectors, such as the S3 and JDBC connectors, allow you to directly specify certain server properties via custom options in the `CREATE EXTERNAL TABLE` command `LOCATION` clause. A `<greenplum_user_name>-user.xml` file specifies property settings for an external data store that are specific to a Greenplum Database user.

For a given Greenplum Database user, PXF uses the following precedence rules (highest to lowest) to obtain configuration property settings for the user:

1. A property that you configure in `<server_name>/<greenplum_user_name>-user.xml` overrides any setting of the property elsewhere.
2. A property that is specified via custom options in the `CREATE EXTERNAL TABLE` command `LOCATION` clause overrides any setting of the property in a PXF server configuration.
3. Properties that you configure in the `<server_name>` PXF server definition identify the default property values.

These precedence rules allow you create a single external table that can be accessed by multiple Greenplum Database users, each with their own unique external data store user credentials.


## <a id="using"></a>Using a Server Configuration

To access an external data store, the Greenplum Database user specifies the server name in the `CREATE EXTERNAL TABLE` command `LOCATION` clause `SERVER=<server_name>` option. The `<server_name>` that the user provides identifies the server configuration directory from which PXF obtains the configuration and credentials to access the external data store.

For example, the following command accesses an S3 object store using the server configuration defined in the `$PXF_BASE/servers/s3srvcfg/s3-site.xml` file:

<pre>
CREATE EXTERNAL TABLE pxf_ext_tbl(name text, orders int)
  LOCATION ('pxf://BUCKET/dir/file.txt?PROFILE=s3:text&<b>SERVER=s3srvcfg</b>')
FORMAT 'TEXT' (delimiter=E',');
</pre>

PXF automatically uses the `default` server configuration when no `SERVER=<server_name>` setting is provided.

For example, if the `default` server configuration identifies a Hadoop cluster, the following example command references the HDFS file located at `/path/to/file.txt`:

<pre>
CREATE EXTERNAL TABLE pxf_ext_hdfs(location text, miles int)
  LOCATION ('pxf://path/to/file.txt?PROFILE=hdfs:text')
FORMAT 'TEXT' (delimiter=E',');
</pre>

<div class="note info"><b>Note:</b> A Greenplum Database user who queries or writes to an external table accesses the external data store with the credentials configured for the <code>&lt;server_name></code> user. If no user-specific credentials are configured for <code>&lt;server_name></code>, the Greenplum user accesses the external data store with the default credentials configured for <code>&lt;server_name></code>.</div>

