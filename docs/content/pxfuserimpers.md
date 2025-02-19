---
title: Configuring the Hadoop User, User Impersonation, and Proxying
---

PXF accesses Hadoop services on behalf of Greenplum Database end users. Impersonation is a way to present a Greenplum end user identity to a remote system. You can achieve this with PXF by configuring a Hadoop proxy user. When the Hadoop service is secured with Kerberos, you also have the option of impersonation using Kerberos constrained delegation.

When user impersonation is activated (the default), PXF accesses non-secured Hadoop services using the identity of the Greenplum Database user account that logs in to Greenplum and performs an operation that uses a PXF connector. Keep in mind that PXF uses only the _login_ identity of the user when accessing Hadoop services. For example, if a user logs in to Greenplum Database as the user `jane` and then runs `SET ROLE` or `SET SESSION AUTHORIZATION` to assume a different user identity, all PXF requests still use the identity `jane` to access Hadoop services. When user impersonation is activated, you must explicitly configure each Hadoop data source (HDFS, Hive, HBase) to allow PXF to act as a proxy for impersonating specific Hadoop users or groups.

When user impersonation is deactivated, PXF runs all Hadoop service requests as the PXF process owner (usually `gpadmin`) or the Hadoop user identity that you specify. This behavior provides no means to control access to Hadoop services for different Greenplum Database users. It requires that this user have access to all files and directories in HDFS, and all tables in Hive and HBase that are referenced in PXF external table definitions.

You configure the Hadoop user and PXF user impersonation setting for a server via the `pxf-site.xml` server configuration file. Refer to [About the pxf-site.xml Configuration File](cfg_server.html#pxf-site) for more information about the configuration properties in this file.

## <a id="pxf_cfg_scenarios"></a>Use Cases and Configuration Scenarios

User, user impersonation, and proxy configuration for Hadoop depends on how you use PXF to access Hadoop, and whether or not the Hadoop cluster is secured with Kerberos.

The following scenarios describe the use cases and configuration required when
you use PXF to access *non-secured Hadoop*.
If you are using PXF to access a *Kerberos-secured Hadoop cluster*, refer to the [Use Cases and Configuration Scenarios](pxf_kerbhdfs.html#scenarios) section in the *Configuring PXF for Secure HDFS* topic.

**Note**: These scenarios assume that `gpadmin` is the PXF process owner.

### <a id="default_case"></a>Accessing Hadoop as the Greenplum User Proxied by gpadmin

This is the default configuration for PXF. The `gpadmin` user proxies Greenplum
queries on behalf of Greenplum users. The effective user in Hadoop is the
Greenplum user that runs the query.

![Accessing Hadoop as the Greenplum User Proxied by gpadmin](graphics/impersonation_cases/impersonation-case-1.png "Accessing Hadoop as the Greenplum User Proxied by gpadmin")

The following table identifies the `pxf.service.user.impersonation` and `pxf.service.user.name` settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  true           |  `gpadmin`        | None; this is the default configuration. | Set the `gpadmin` user as the Hadoop proxy user as described in [Configure Hadoop Proxying](#hadoop). |


### <a id="custom_proxy"></a>Accessing Hadoop as the Greenplum User Proxied by a &lt;custom> User

In this configuration, PXF accesses Hadoop as the Greenplum user proxied
by \<custom> user. A query initiated by a Greenplum user appears on the Hadoop side
as originating from the (\<custom> user.

This configuration might be desirable when Hadoop is already configured with a
proxy user, or when you want a user different than `gpadmin` to proxy Greenplum queries.

![Accessing Hadoop as the Greenplum User Proxied by a custom User](graphics/impersonation_cases/impersonation-case-2.png "Accessing Hadoop as the Greenplum User Proxied by a custom User")

The following table identifies the `pxf.service.user.impersonation` and `pxf.service.user.name` settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  true          |  \<custom>        | [Configure the Hadoop User](#pxf_cfg_user) to the \<custom> user name. | Set the \<custom>  user as the Hadoop proxy user as described in [Configure Hadoop Proxying](#hadoop). |


### <a id="gpadmin_hadoop"></a>Accessing Hadoop as the gpadmin User

In this configuration, PXF accesses Hadoop as the `gpadmin` user. A query initiated by
any Greenplum user appears on the Hadoop side as originating from the `gpadmin` user.

![Accessing Hadoop as the gpadmin User](graphics/impersonation_cases/impersonation-case-3.png "Accessing Hadoop as the gpadmin User")

The following table identifies the `pxf.service.user.impersonation` and `pxf.service.user.name` settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  false         |  `gpadmin`        | Turn off user impersonation as described in [Configure PXF User Impersonation](#pxf_cfg_impers). | None required. |

### <a id="custom_hadoop"></a>Accessing Hadoop as a &lt;custom> User

In this configuration, PXF accesses Hadoop as a \<custom> user. A query initiated by
any Greenplum user appears on the Hadoop side as originating from the \<custom> user.

![Accessing Hadoop as a custom User](graphics/impersonation_cases/impersonation-case-4.png "Accessing Hadoop as a custom User")

The following table identifies the `pxf.service.user.impersonation` and `pxf.service.user.name` settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  false         |  \<custom>        |  Turn off user impersonation as described in [Configure PXF User Impersonation](#pxf_cfg_impers) and [Configure the Hadoop User](#pxf_cfg_user) to the \<custom> user name. | None required. |


## <a id="pxf_cfg_user"></a>Configure the Hadoop User

By default, PXF accesses Hadoop using the identity of the Greenplum Database user. You can configure PXF to access Hadoop as a different user on a per-server basis.

Perform the following procedure to configure the Hadoop user:

1. Log in to your Greenplum Database coordinator host as the administrative user:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Identify the name of the Hadoop PXF server configuration that you want to update.

3. Navigate to the server configuration directory. For example, if the server is named `hdp3`:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

4. If the server configuration does not yet include a `pxf-site.xml` file, copy the template file to the directory. For example:

    ``` shell
    gpadmin@coordinator$ cp <PXF_INSTALL_DIR>/templates/pxf-site.xml .
    ```

5. Open the `pxf-site.xml` file in the editor of your choice, and configure the Hadoop user name. When impersonation is deactivated, this name identifies the Hadoop user identity that PXF will use to access the Hadoop system. When user impersonation is activated for a non-secure Hadoop cluster, this name identifies the PXF proxy Hadoop user. For example, if you want to access Hadoop as the user `hdfsuser1`, uncomment the property and set it as follows:

    ``` xml
    <property>
        <name>pxf.service.user.name</name>
        <value>hdfsuser1</value>
    </property>
    ```

    The Hadoop user `hdfsuser1` must exist in the Hadoop cluster.

7. Save the `pxf-site.xml` file and exit the editor.

8. Use the `pxf cluster sync` command to synchronize the PXF Hadoop server configuration to your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```


## <a id="pxf_cfg_impers"></a>Configure PXF User Impersonation

PXF user impersonation is activated by default for Hadoop servers. You can configure PXF user impersonation on a per-server basis. Perform the following procedure to turn PXF user impersonation on or off for the Hadoop server configuration:

1. Navigate to the server configuration directory. For example, if the server is named `hdp3`:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

2. If the server configuration does not yet include a `pxf-site.xml` file, copy the template file to the directory. For example:

    ``` shell
    gpadmin@coordinator$ cp <PXF_INSTALL_DIR>/templates/pxf-site.xml .
    ```

3. Open the `pxf-site.xml` file in the editor of your choice, and update the user impersonation property setting. For example, if you do not require user impersonation for this server configuration, set the `pxf.service.user.impersonation` property to `false`:

    ``` xml
    <property>
        <name>pxf.service.user.impersonation</name>
        <value>false</value>
    </property>
    ```

    If you require user impersonation, turn it on:

    ``` xml
    <property>
        <name>pxf.service.user.impersonation</name>
        <value>true</value>
    </property>
    ```

3. If you activated user impersonation and Kerberos constrained delegation is deactivated (the default), you must configure Hadoop proxying as described in [Configure Hadoop Proxying](#hadoop). You must also configure [Hive User Impersonation](#hive) and [HBase User Impersonation](#hbase) if you plan to use those services.

4. Save the `pxf-site.xml` file and exit the editor.

5. Use the `pxf cluster sync` command to synchronize the PXF Hadoop server configuration to your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```


## <a id="hadoop"></a>Configure Hadoop Proxying

When PXF user impersonation is activated for a Hadoop server configuration and Kerberos constrained delegation is deactivated (the default), you must configure Hadoop to permit PXF to proxy Greenplum users. This configuration involves setting certain `hadoop.proxyuser.*` properties. Follow these steps to set up PXF Hadoop proxy users:

1. Log in to your Hadoop cluster and open the `core-site.xml` configuration file using a text editor, or use Ambari or another Hadoop cluster manager to add or edit the Hadoop property values described in this procedure.

2. Set the property `hadoop.proxyuser.<name>.hosts` to specify the list of PXF host names from which proxy requests are permitted. Substitute the PXF proxy Hadoop user for `<name>`. The PXF proxy Hadoop user is the `pxf.service.user.name` that you configured in the procedure above, or, if you are using Kerberos authentication to Hadoop, the proxy user identity is the *primary* component of the Kerberos principal. If you have not explicitly configured `pxf.service.user.name`, the proxy user is the operating system user that started PXF. Provide multiple PXF host names in a comma-separated list. For example, if the PXF proxy user is named `hdfsuser2`:

    ``` xml
    <property>
        <name>hadoop.proxyuser.hdfsuser2.hosts</name>
        <value>pxfhost1,pxfhost2,pxfhost3</value>
    </property>
    ```

3. Set the property `hadoop.proxyuser.<name>.groups` to specify the list of HDFS groups that PXF as Hadoop user `<name>` can impersonate. You should limit this list to only those groups that require access to HDFS data from PXF.  For example:

    ``` xml
    <property>
        <name>hadoop.proxyuser.hdfsuser2.groups</name>
        <value>group1,group2</value>
    </property>
    ```

4. You must restart Hadoop for your `core-site.xml` changes to take effect.

5. Copy the updated `core-site.xml` file to the PXF Hadoop server configuration directory `$PXF_BASE/servers/<server_name>` on the Greenplum Database coordinator host and synchronize the configuration to the standby coordinator host and each Greenplum Database segment host.

## <a id="hive"></a>Hive User Impersonation

The PXF Hive connector uses the Hive MetaStore to determine the HDFS locations of Hive tables, and then accesses the underlying HDFS files directly. No specific impersonation configuration is required for Hive, because the Hadoop proxy configuration in `core-site.xml` also applies to Hive tables accessed in this manner.


## <a id="hbase"></a>HBase User Impersonation

In order for user impersonation to work with HBase, you must activate the `AccessController` coprocessor in the HBase configuration and restart the cluster. See [61.3 Server-side Configuration for Simple User Access Operation](http://hbase.apache.org/book.html#hbase.secure.configuration) in the Apache HBase Reference Guide for the required `hbase-site.xml` configuration settings.

