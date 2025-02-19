---
title: Upgrading from an Earlier Version 6 Release
---

If you have installed a PXF 6.x `rpm` or `deb` package and have configured and are using PXF in your current Greenplum Database 5.21.2+ or 6.x installation, you must perform some upgrade actions when you install a new version of PXF 6.x.

The PXF upgrade procedure has three steps. You perform one pre-install procedure, the install itself, and then a post-install procedure to upgrade to PXF 6.x:

-   [Step 1: Perform the PXF Pre-Upgrade Actions](#pxfpre)
-   [Step 2: Install the New PXF 6.x](#pxfinst)
-   [Step 3: Complete the Upgrade to a Newer PXF 6.x](#pxfup)


## <a id="pxfpre"></a>Step 1: Perform the PXF Pre-Upgrade Actions

Perform this procedure before you upgrade to a new version of PXF 6.x:

1. Log in to the Greenplum Database coordinator host. For example:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. Identify and note the version of PXF currently running in your Greenplum cluster:

    ``` shell
    gpadmin@coordinator$ pxf version
    ```

1. Stop PXF on each Greenplum host as described in [Stopping PXF](cfginitstart_pxf.html#stop_pxf):

    ``` shell
    gpadmin@coordinator$ pxf cluster stop
    ```

1. (Optional, Recommended) Back up the PXF user configuration files; for example, if `PXF_BASE=/usr/local/pxf-gp6`:

    ``` shell
    gpadmin@coordinator$ cp -avi /usr/local/pxf-gp6 pxf_base.bak
    ```


## <a id="pxfinst"></a>Step 2: Installing the New PXF 6.x

Install PXF 6.x and identify and note the new PXF version number.


## <a id="pxfup"></a>Step 3: Completing the Upgrade to a Newer PXF 6.x

After you install the new version of PXF, perform the following procedure:

1. Log in to the Greenplum Database coordinator host. For example:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. PXF 6.x includes a new version of the `pxf` extension. Register the extension files with Greenplum Database (see [pxf cluster register](ref/pxf-cluster.html)). `$GPHOME` must be set when you run this command:

    ``` shell
    gpadmin@coordinator$ pxf cluster register
    ```

1. You must update the `pxf` extension in every Greenplum database in which you are using PXF. A database superuser or the database owner must run this SQL command in the `psql` subsystem or in an SQL script:

    ``` sql
    ALTER EXTENSION pxf UPDATE;
    ```

1. **If you are upgrading <i>from</i> PXF version 6.0.x**:
    - If you previously set the `pxf.connection.timeout` property to change the write/upload timeout, you must now set the `pxf.connection.upload-timeout` property for this purpose.
    - Existing external tables that access Avro arrays and JSON objects will continue to work as-is. If you want to take advantage of the new Avro array read/write functionality or the new JSON object support, create a new external table with the adjusted DDL. If you can access the data with the new external table as you expect, you may choose to drop and recreate the existing external table.

1. **If you are upgrading <i>to</i> PXF version 6.2.0 to resolve an erroneous replay attack issue in a Kerberos-secured environment**:

    1. If you want to change the default value of the new `pxf.sasl.connection.retries` property, add the following to the `pxf-site.xml` file for your PXF server:

        ``` pre
        <property>
          <name>pxf.sasl.connection.retries</name>
          <value><new-value></value>
          <description>
            Specifies the number of retries to perform when a SASL connection is refused by a Namenode due to 'GSS initiate failed' error.
          </description>
        </property>
        ```

    1. (Recommended) Configure PXF to use a host-specific Kerberos principal for each segment host. If you specify the following `pxf.service.kerberos.principal` property setting in the PXF server's `pxf-site.xml` file, PXF automatically replaces ` _HOST` with the FQDN of the segment host:

        ``` pre
        <property>
          <name>pxf.service.kerberos.principal</name>
          <value>gpadmin/_HOST@REALM.COM</value>
        </property>
        ```

1. (Recommended) **If you are upgrading <i>from</i> PXF version 6.2.2 or earlier <i>to</i> PXF version 6.2.3 or later**, update your `$PXF_BASE/conf/pxf-log4j2.xml` file to fully configure the logging changes introduced in version 6.2.3:

    1. Remove the following line from the initial `<Properties>` block:
    
        ``` pre
        <Property name="PXF_LOG_LEVEL">${bundle:pxf-application:pxf.log.level}</Property>
        ```

    1. Change the following line:

        ``` pre
        <Logger name="org.greenplum.pxf" level="${env:PXF_LOG_LEVEL:-${sys:PXF_LOG_LEVEL:-info}}"/>
        ```

        to:

        ``` pre
        <Logger name="org.greenplum.pxf" level="${env:PXF_LOG_LEVEL:-${spring:pxf.log.level}}"/>
        ```

1. **If you are upgrading <i>from</i> PXF version 6.5.x or earlier <i>to</i> PXF version 6.6.0 or later**, and you want to change the value of the new `pxf.parquet.write.decimal.overflow` property from the default of `round`, add the following to the `pxf-site.xml` file for your PXF server:

    ``` pre
    <property>
      <name>pxf.parquet.write.decimal.overflow</name>
      <value>NEW_VALUE</value>
    </property>
    ```

    where `NEW_VALUE` is `error` or `ignore`.

    Refer to [Understanding Overflow Conditions When Writing Numeric Data](hdfs_parquet.html#overflow) for more information about how PXF uses this property to direct its actions when it encounters a numeric overflow while writing to a Parquet file.

1. **If you are upgrading <i>from</i> PXF version 6.6.x or earlier <i>to</i> PXF version 6.7.0 or later**, and you want to change the value of the new `pxf.orc.write.decimal.overflow` property from the default of `round`, add the following to the `pxf-site.xml` file for your PXF server:

    ``` pre
    <property>
      <name>pxf.orc.write.decimal.overflow</name>
      <value>NEW_VALUE</value>
    </property>
    ```

    where `NEW_VALUE` is `error` or `ignore`.

    Refer to [Understanding Overflow Conditions When Writing Numeric Data](hdfs_orc.html#overflow) for more information about how PXF uses this property to direct its actions when it encounters a numeric overflow while writing to an ORC file.

1. **If you are upgrading <i>from</i> PXF version 6.6.x or earlier <i>to</i> PXF version 6.7.0 or later**, and you want to retain the previous PXF listen address (`0.0.0.0`), change the value of the `server.address` `pxf-application.properties` property as described in [Configuring the Listen Address](cfghostport.html#listen_address).

1. **If you are upgrading <i>from</i> PXF version 6.7.x or earlier <i>to</i> PXF version 6.8.0 or later** and you want to change the default value of the new `pxf.service.kerberos.ticket-renew-window` property, add the following to the `pxf-site.xml` file for your PXF server:

        ``` pre
        <property>
          <name>pxf.service.kerberos.ticket-renew-window</name>
          <value><new-value></value>
          <description>
            By default, PXF refreshes the Kerberos ticket when 80% (0.8) of its lifespan has elapsed.
            Specifies the window, as a float, at which PXF will actively refresh the ticket. This value
            must be between 0 and 1. Setting this value to 0 means that every PXF request will get a new
            ticket.
          </description>
        </property>
        ```

1. Synchronize the PXF configuration from the coordinator host to the standby coordinator host and each Greenplum Database segment host. For example:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```
 
1. Start PXF on each Greenplum host as described in [Starting PXF](cfginitstart_pxf.html#start_pxf):

    ``` shell
    gpadmin@coordinator$ pxf cluster start
    ```
 
