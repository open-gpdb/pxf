---
title: Upgrading PXF
---

If you have installed the PXF `rpm` or `deb` package and have initialized, configured, and are using PXF in your current Greenplum Database 5.21.2+ or 6.x installation, you must perform some upgrade actions when you install a new version of PXF.

The PXF upgrade procedure has two parts. You perform one procedure before, and one procedure after, you install a new version to upgrade PXF:

-   [Step 1: Complete the PXF Pre-Upgrade Actions](#pxfpre)
-   Install the new version of PXF
-   [Step 2: Upgrade PXF](#pxfup)


## <a id="pxfpre"></a>Step 1: Complete the PXF Pre-Upgrade Actions

Perform this procedure before you upgrade to a new version of PXF:

1. Log in to the Greenplum Database master host. For example:

    ``` shell
    $ ssh gpadmin@<gpmaster>
    ```

2. Identify and note the version of PXF currently running in your Greenplum cluster:

    ``` shell
    gpadmin@gpmaster$ pxf version
    ```

2. If the `$GPHOME/pxf` directory exists, and you are running PXF version 5.12.x or older, back up the Greenplum PXF embedded installation. For example:

    ``` shell
    gpadmin@gpmaster$ mkdir $HOME/pxf_gp_backup
    gpadmin@gpmaster$ cp -r $GPHOME/pxf $HOME/pxf_gp_backup/
    gpadmin@gpmaster$ cp $GPHOME/share/postgresql/extension/pxf* $HOME/pxf_gp_backup/
    gpadmin@gpmaster$ cp $GHOME/lib/postgresql/pxf* $HOME/pxf_gp_backup/
    ```

2. Stop PXF on each segment host as described in [Stopping PXF](cfginitstart_pxf.html#stop_pxf).

3. Install the new version of PXF, identify and note the new PXF version number, and then continue your PXF upgrade with [Step 2: Completing the PXF Upgrade](#pxfup).


## <a id="pxfup"></a>Step 2: Upgrade PXF

After you install the new version of PXF, perform the following procedure:

1. Log in to the Greenplum Database master host. For example:

    ``` shell
    $ ssh gpadmin@<gpmaster>
    ```

2. Initialize PXF on each segment host as described in [Initializing PXF](init_pxf.html). You may choose to use your existing `$PXF_CONF` for the initialization.

3. **If you are upgrading from PXF version 5.9.x or earlier** and you have configured any JDBC servers that access Kerberos-secured Hive, you must now set the `hadoop.security.authentication` property to the `jdbc-site.xml` file to explicitly identify use of the Kerberos authentication method. Perform the following for each of these server configs:

    1. Navigate to the server configuration directory.
    2. Open the `jdbc-site.xml` file in the editor of your choice and uncomment or add the following property block to the file:

        ```xml
        <property>
            <name>hadoop.security.authentication</name>
            <value>kerberos</value>
        </property>
        ```
    3. Save the file and exit the editor.

4. **If you are upgrading from PXF version 5.11.x or earlier**: The PXF `Hive` and `HiveRC` profiles now support column projection using column name-based mapping. If you have any existing PXF external tables that specify one of these profiles, and the external table relied on column index-based mapping, you may be required to drop and recreate the tables:

    1. Identify all PXF external tables that you created that specify a `Hive` or `HiveRC` profile.

    2. For *each* external table that you identify in step 1, examine the definitions of both the PXF external table and the referenced Hive table. If the column names of the PXF external table *do not* match the column names of the Hive table:

        1. Drop the existing PXF external table. For example:

            ``` sql
            DROP EXTERNAL TABLE pxf_hive_table1;
            ```

        2. Recreate the PXF external table using the Hive column names. For example:

            ``` sql
            CREATE EXTERNAL TABLE pxf_hive_table1( hivecolname int, hivecolname2 text )
              LOCATION( 'pxf://default.hive_table_name?PROFILE=Hive')
            FORMAT 'custom' (FORMATTER='pxfwritable_import');
            ```

        3. Review any SQL scripts that you may have created that reference the PXF external table, and update column names if required.

4. **If you are upgrading from PXF version 5.15.x or earlier**:

    1. The `pxf.service.user.name` property in the `pxf-site.xml` template file is now commented out by default. Keep this in mind when you configure new PXF servers.
    2. The default value for the `jdbc.pool.property.maximumPoolSize` property is now `15`. If you have previously configured a JDBC server(s) and want that server to use the new default value, you must manually change the property value in the server's `jdbc-site.xml` file.
    3. PXF 5.16 disallows specifying relative paths and environment variables in the `CREATE EXTERNAL TABLE` `LOCATION` clause file path. If you previously created any external tables that specified a relative path or environment variable, you must drop each external table, and then re-create it without these constructs.
    4. Filter pushdown is activated by default for queries on external tables that specify the `Hive`, `HiveRC`, or `HiveORC` profiles. If you have previously created an external table that specifies one of these profiles and queries are failing with PXF v5.16+, you can deactivate filter pushdown at the external table-level or at the server level:

        1. (External table) Drop the external table and re-create it, specifying the `&PPD=false` option in the `LOCATION` clause.
        2. (Server) If you do not want to recreate the external table, you can deactivate filter pushdown *for all* `Hive*` *profile queries using the server* by setting the `pxf.ppd.hive` property in the `pxf-site.xml` file to `false`:

            ``` pre
            <property>
                <name>pxf.ppd.hive</name>
                <value>false</value>
            </property>
            ```

            You may need to add this property block to the `pxf-site.xml` file.

4. Synchronize the PXF configuration from the master host to the standby master host and each Greenplum Database segment host. For example:

    ``` shell
    gpadmin@gpmaster$ pxf cluster sync
    ```
 
5. Start PXF on each segment host as described in [Starting PXF](cfginitstart_pxf.html#start_pxf).

