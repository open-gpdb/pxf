---
title: Configuring for Secure HDFS
---

When Kerberos is activated for your HDFS filesystem, the PXF Service, as an HDFS client, requires a principal and keytab file to authenticate access to HDFS. To read or write files on a secure HDFS, you must create and deploy Kerberos principals and keytabs for PXF, and ensure that Kerberos authentication is activated and functioning.

PXF accesses a secured Hadoop cluster on behalf of Greenplum Database end users. Impersonation is a way to present a Greenplum end user identity to a remote system. You can achieve this on a secured Hadoop cluster with PXF by configuring a Hadoop proxy user or using Kerberos constrained delegation.

The identity with which PXF accesses a Kerberos-secured Hadoop depends on the settings of the following properties:

| Property       |  Description | Default Value |
|----------------|--------------|---------------|
| pxf.service.kerberos.principal | The PXF Kerberos principal name. | gpadmin/\_HOST@EXAMPLE.COM |
| pxf.service.user.impersonation | Activates/deactivates Greenplum Database user impersonation on the remote system. | `true` |
| pxf.service.kerberos.constrained-delegation | Activates/deactivates usage of Kerberos constrained delegation based on S4U Kerberos extensions. This option allows Hadoop administrators to avoid creating a proxy user configuration in Hadoop, instead requiring them to perform delegation configuration in an Active Directory (AD) or Identity Policy Audit (IPA) server. | `false` |
| pxf.service.kerberos.ticket-renew-window| The minimum elapsed lifespan (as a percentage) after which PXF attempts to renew/refresh a Kerberos ticket. Value range is from 0 (PXF generates a new ticket for all requests) to 1 (PXF renews after full ticket lifespan). | 0.8 (80%) |
| pxf.service.user.name | (Optional) The user name with which PXF connects to a remote Kerberos-secured cluster if user impersonation is deactivated and using the `pxf.service.kerberos.principal` is not desired. | None |

You configure these setting for a Hadoop PXF server via the `pxf-site.xml` configuration file. Refer to [About the pxf-site.xml Configuration File](cfg_server.html#pxf-site) for more information about the configuration properties in this file.

**Note:** PXF supports simultaneous access to multiple Kerberos-secured Hadoop clusters.

## <a id="kcd"></a>About Kerberos Constrained Delegation

Kerberos constrained delegation is a feature that allows an administrator to specify trust boundaries that restrict the scope of where an application can act on behalf of a user. You may choose to configure PXF to use Kerberos constrained delegation when you want to manage user impersonation privileges in a directory service without the need to specify a proxy Hadoop user. Refer to the Microsoft [Service for User](https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-sfu/bde93b0e-f3c9-4ddf-9f44-e1453be7af5a) (S4U) Kerberos protocol extension documentation for more information about Kerberos constrained delegation.

When your AD or IPA server is configured appropriately and you activate Kerberos constrained delegation for PXF, the PXF service requests and obtains a Kerberos ticket on behalf of the user, and uses the ticket to access the HDFS file system. PXF caches the ticket for one day.

PXF supports Kerberos Constrained Delegation only when you use the `hdfs:*` or `hive:*` profiles to access data residing in a Kerberos-secured Hadoop cluster.

By default, Kerberos constrained delegation is deactivated for PXF. To activate Kerberos constrained delegation for a specific PXF server, you must set `pxf.service.kerberos-constrained.delegation` to `true` in the server's `pxf-site.xml` configuration file.

## <a id="prereq"></a>Prerequisites

Before you configure PXF for access to a secure HDFS filesystem, ensure that you have:

- Identified whether or not you plan to have PXF use Kerberos constrained delegation to access Hadoop.

- Configured a PXF server for the Hadoop cluster, and can identify the server configuration name.

- Configured and started PXF as described in [Configuring PXF](instcfg_pxf.html).

- Verified that Kerberos is activated for your Hadoop cluster.

- Verified that the HDFS configuration parameter `dfs.block.access.token.enable` is set to `true`. You can find this setting in the `hdfs-site.xml` configuration file on a host in your Hadoop cluster.

- Noted the host name or IP address of each Greenplum Database host (\<gphost\>) and the Kerberos Key Distribution Center \(KDC\) \<kdc-server\> host.

- Noted the name of the Kerberos \<realm\> in which your cluster resides.

- Installed the Kerberos client packages on **each** Greenplum Database host if they are not already installed. You must have superuser permissions to install operating system packages. For example:

    ``` shell
    root@gphost$ rpm -qa | grep krb
    root@gphost$ yum install krb5-libs krb5-workstation
    ```

Ensure that you meet these additional prerequisites when PXF uses Kerberos constrained delegation:

- S4U is activated in the AD or IPA server.

- The AD or IPA server is configured to allow the PXF Kerberos principal to impersonate end users.

## <a id="scenarios"></a>Use Cases and Configuration Scenarios

The following scenarios describe the use cases and configuration required when
you use PXF to access a Kerberos-secured Hadoop cluster.

**Note**: These scenarios assume that `gpadmin` is the PXF process owner.

### <a id="principal_proxy"></a>Accessing Hadoop as the Greenplum User

#### Proxied by the Kerberos Principal

In this configuration, PXF accesses Hadoop as the Greenplum user proxied
by the Kerberos principal. The Kerberos principal is the Hadoop proxy user and
accesses Hadoop as the Greenplum user.

This is the default configuration for a Hadoop PXF server.

![Accessing Hadoop as the Greenplum User Proxied by the Kerberos Principal](graphics/impersonation_cases/impersonation-case-1-kerberos.png "Accessing Hadoop as the Greenplum User Proxied by the Kerberos Principal")

The following table identifies the impersonation and service user settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  true          | Greenplum user | Perform the [Configuration Procedure](#procedure) in this topic. | Set the Kerberos principal as the Hadoop proxy user as described in [Configure Hadoop Proxying](pxfuserimpers.html#hadoop). |

#### Using Kerberos Constrained Delegation

In this configuration, PXF uses Kerberos constrained delegation to request and obtain a ticket on behalf of the Greenplum user, and uses the ticket to access Hadoop.

![Accessing Hadoop using Kerberos Constrained Delegation](graphics/impersonation_cases/impersonation-case-kcd.png "Accessing Hadoop using Kerberos Constrained Delegation")

The following table identifies the impersonation and service user settings, and the PXF and directory service configuration required for this use case; no Hadoop configuration is required:

| Impersonation  | Service&nbsp;User |  PXF Configuration | AD/IPA&nbsp;Config&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  true          | Greenplum user | Set up the PXF Kerberos principal, keytab files, and related settings in `pxf-site.xml` as described in the [Configuration Procedure](#procedure) in this topic, and [Activate Kerberos Constrained Delegation](#enable_kcd). | Configure AD or IPA to provide the PXF Kerberos principal with the delegation rights for the Greenplum end users. |

### <a id="principal_hadoop"></a>Accessing Hadoop as the Kerberos Principal

In this configuration, PXF accesses Hadoop as the Kerberos principal. A query initiated by
any Greenplum user appears on the Hadoop side as originating from the Kerberos principal.

![Accessing Hadoop as the Kerberos Principal](graphics/impersonation_cases/impersonation-case-2-kerberos.png "Accessing Hadoop as the Kerberos Principal")

The following table identifies the impersonation and service user settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  false         | Identity of the Kerberos principal | Perform the [Configuration Procedure](#procedure.html) in this topic, and then turn off user impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers). | None required. |

### <a id="custom_hadoop"></a>Accessing Hadoop as a &lt;custom> User

#### Proxied by the Kerberos Principal

In this configuration, PXF accesses Hadoop as a \<custom> user (for example, `hive`).
The Kerberos principal is the Hadoop proxy user. A query initiated by any Greenplum
user appears on the Hadoop side as originating from the \<custom> user.

![Accessing Hadoop as a custom User](graphics/impersonation_cases/impersonation-case-3-kerberos.png "Accessing Hadoop as a custom User")

The following table identifies the impersonation and service user settings, and the PXF and Hadoop configuration required for this use case:

| Impersonation  | Service&nbsp;User |  PXF Configuration | Hadoop&nbsp;Configuration&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  false         |  \<custom>        | Perform the [Configuration Procedure](#procedure) in this topic, turn off user impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers), and [Configure the Hadoop User](pxfuserimpers.html#pxf_cfg_user) to the \<custom> user name. | Set the Kerberos principal as the Hadoop proxy user as described in [Configure Hadoop Proxying](pxfuserimpers.html#hadoop). |

**Note:** PXF does not support accessing a Kerberos-secured Hadoop cluster
with a &lt;custom> user impersonating Greenplum users. PXF requires that you
impersonate Greenplum users using the Kerberos principal.

#### Using Kerberos Constrained Delegation

In this configuration, PXF uses Kerberos constrained delegation to request and obtain a ticket on behalf of a \<custom> user, and uses the ticket to access Hadoop.

The following table identifies the impersonation and service user settings, and the PXF and directory service configuration required for this use case; no Hadoop configuration is required:

| Impersonation  | Service&nbsp;User |  PXF Configuration | AD/IPA&nbsp;Config&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |
|----------------|-------------------|--------------------|-----------|
|  false          | \<custom\> | Set up the PXF Kerberos principal, keytab files, and related settings in `pxf-site.xml` as described in the [Configuration Procedure](#procedure) in this topic, deactivate impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers), [Activate Kerberos Constrained Delegation](#enable_kcd), and [Configure the Hadoop User](pxfuserimpers.html#pxf_cfg_user) to the \<custom> user name.  | Configure AD or IPA to provide the PXF Kerberos principal with the delegation rights for the \<custom> user name. |



## <a id="procedure"></a>Procedures

There are different procedures for configuring PXF for secure HDFS with a [Microsoft Active Directory KDC Server](#proc_ad) vs. with an [MIT Kerberos KDC Server](#proc_mit).


### <a id="proc_ad"></a>Configuring PXF with a Microsoft Active Directory Kerberos KDC Server

When you configure PXF for secure HDFS using an AD Kerberos KDC server, you will perform tasks on both the KDC server host and the Greenplum Database coordinator host.

**Perform the following steps to configure the Active Directory domain controller**:

1. Start **Active Directory Users and Computers**.
2. Expand the forest domain and the top-level UNIX organizational unit that describes your Greenplum user domain.
3. Select **Service Accounts**, right-click, then select **New->User**.
4. Type a name, for example: `ServiceGreenplumPROD1`, and change the login name to `gpadmin`. Note that the login name should be in compliance with POSIX standard and match `hadoop.proxyuser.<name>.hosts/groups` in the Hadoop `core-site.xml` and the Kerberos principal.
5. Type and confirm the Active Directory service account password. Select the **User cannot change password** and **Password never expires** check boxes, then click **Next**. For security reasons, if you can't have **Password never expires** checked, you will need to generate new keytab file (step 7) every time you change the password of the service account. 
6. Click **Finish** to complete the creation of the new user principal. 
7. Open Powershell or a command prompt and run the `ktpass` command to generate the keytab file. For example:

    ``` shell
    powershell#>ktpass -out pxf.service.keytab -princ gpadmin@EXAMPLE.COM -mapUser ServiceGreenplumPROD1 -pass ******* -crypto all -ptype KRB5_NT_PRINCIPAL
    ```

    With Active Directory, the principal and the keytab file are shared by all Greenplum Database hosts. 
	
8. Copy the `pxf.service.keytab` file to the Greenplum coordinator host.

**Perform the following procedure on the Greenplum Database coordinator host**:

1. Log in to the Greenplum Database coordinator host. For example:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```
    
2. Identify the name of the PXF Hadoop server configuration, and navigate to the server configuration directory. For example, if the server is named `hdp3`:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

3. If the server configuration does not yet include a `pxf-site.xml` file, copy the template file to the directory. For example:

    ``` shell
    gpadmin@coordinator$ cp <PXF_INSTALL_DIR>/templates/pxf-site.xml .
    ```

4. Open the `pxf-site.xml` file in the editor of your choice, and update the keytab and principal property settings, if required. Specify the location of the keytab file and the Kerberos principal, substituting your realm. For example:

    ``` xml
    <property>
        <name>pxf.service.kerberos.principal</name>
        <value>gpadmin@EXAMPLE.COM</value>
    </property>
    <property>
        <name>pxf.service.kerberos.keytab</name>
        <value>${pxf.conf}/keytabs/pxf.service.keytab</value>
    </property>
    ```

4. Save the file and exit the editor.

5. Synchronize the keytabs in `$PXF_BASE`. You must distribute the keytab file to `$PXF_BASE/keytabs/`. Locate the keytab file and copy the file to the `$PXF_BASE` runtime configuration directory. The copy command that you specify differs based on the Greenplum Database version. For example:

    If your source Greenplum cluster is running version 5.x or 6.x:
	
    ``` shell
    gpadmin@coordinator$ gpscp -f hostfile_all pxf.service.keytab =:$PXF_BASE/keytabs/
    ```

    If your source Greenplum cluster is running version 7.x:

    ``` shell
    gpadmin@coordinator$ gpsync -f hostfile_all pxf.service.keytab =:$PXF_BASE/keytabs/
    ```

5. Set the required permissions on the keytab file. For example:

    ``` shell
    gpadmin@coordinator$ gpssh -f hostfile_all chmod 400 $PXF_BASE/keytabs/pxf.service.keytab
    ```

6. [Complete the PXF Configuration](#proc_complete) based on your chosen Hadoop access scenario.

### <a id="proc_mit"></a>Configuring PXF with an MIT Kerberos KDC Server

When you configure PXF for secure HDFS using an MIT Kerberos KDC server, you will perform tasks on both the KDC server host and the Greenplum Database coordinator host.

**Perform the following steps on the MIT Kerberos KDC server host**:

1.  Log in to the Kerberos KDC server as the `root` user.

    ``` shell
    $ ssh root@<kdc-server>
    root@kdc-server$ 
    ```

2. Distribute the `/etc/krb5.conf` Kerberos configuration file on the KDC server host to **each** host in your Greenplum Database cluster if not already present. For example:

    ``` shell
    root@kdc-server$ scp /etc/krb5.conf <gphost>:/etc/krb5.conf
    ```

3.  Use the `kadmin.local` command to create a Kerberos PXF Service principal for **each** Greenplum Database host. The service principal should be of the form `gpadmin/<gphost>@<realm>` where \<gphost\> is the DNS resolvable, fully-qualified hostname of the host system \(output of the `hostname -f` command\).

    For example, these commands create Kerberos PXF Service principals for the hosts named host1.example.com, host2.example.com, and host3.example.com in the Kerberos realm named `EXAMPLE.COM`:

    ``` shell
    root@kdc-server$ kadmin.local -q "addprinc -randkey -pw changeme gpadmin/host1.example.com@EXAMPLE.COM"
    root@kdc-server$ kadmin.local -q "addprinc -randkey -pw changeme gpadmin/host2.example.com@EXAMPLE.COM"
    root@kdc-server$ kadmin.local -q "addprinc -randkey -pw changeme gpadmin/host3.example.com@EXAMPLE.COM"
    ```

4.  Generate a keytab file for each PXF Service principal that you created in the previous step. Save the keytab files in any convenient location (this example uses the directory `/etc/security/keytabs`). You will deploy the keytab files to their respective Greenplum Database host machines in a later step. For example:

    ``` shell
    root@kdc-server$ kadmin.local -q "xst -norandkey -k /etc/security/keytabs/pxf-host1.service.keytab gpadmin/host1.example.com@EXAMPLE.COM"
    root@kdc-server$ kadmin.local -q "xst -norandkey -k /etc/security/keytabs/pxf-host2.service.keytab gpadmin/host2.example.com@EXAMPLE.COM"
    root@kdc-server$ kadmin.local -q "xst -norandkey -k /etc/security/keytabs/pxf-host3.service.keytab gpadmin/host3.example.com@EXAMPLE.COM"
    ```

    Repeat the `xst` command as necessary to generate a keytab for each PXF Service principal that you created in the previous step.

5.  List the principals. For example:

    ``` shell
    root@kdc-server$ kadmin.local -q "listprincs"
    ```

6.  Copy the keytab file for each PXF Service principal to its respective host. For example, the following commands copy each principal generated in step 4 to the PXF default keytab directory on the host when `PXF_BASE=/usr/local/pxf-gp6`:

    ``` shell
    root@kdc-server$ scp /etc/security/keytabs/pxf-host1.service.keytab host1.example.com:/usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ scp /etc/security/keytabs/pxf-host2.service.keytab host2.example.com:/usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ scp /etc/security/keytabs/pxf-host3.service.keytab host3.example.com:/usr/local/pxf-gp6/keytabs/pxf.service.keytab
    ```

    Note the file system location of the keytab file on each PXF host; you will need this information for a later configuration step.

7. Change the ownership and permissions on the `pxf.service.keytab` files. The files must be owned and readable by only the `gpadmin` user. For example:

    ``` shell 
    root@kdc-server$ ssh host1.example.com chown gpadmin:gpadmin /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ ssh host1.example.com chmod 400 /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ ssh host2.example.com chown gpadmin:gpadmin /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ ssh host2.example.com chmod 400 /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ ssh host3.example.com chown gpadmin:gpadmin /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    root@kdc-server$ ssh host3.example.com chmod 400 /usr/local/pxf-gp6/keytabs/pxf.service.keytab
    ```

**Perform the following steps on the Greenplum Database coordinator host**:

1. Log in to the coordinator host. For example:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Identify the name of the PXF Hadoop server configuration that requires Kerberos access.

3. Navigate to the server configuration directory. For example, if the server is named `hdp3`:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

4. If the server configuration does not yet include a `pxf-site.xml` file, copy the template file to the directory. For example:

    ``` shell
    gpadmin@coordinator$ cp <PXF_INSTALL_DIR>/templates/pxf-site.xml .
    ```

5. Open the `pxf-site.xml` file in the editor of your choice, and update the keytab and principal property settings, if required. Specify the location of the keytab file and the Kerberos principal, substituting your realm. *The default values for these settings are identified below*:

    ``` xml
    <property>
        <name>pxf.service.kerberos.principal</name>
        <value>gpadmin/_HOST@EXAMPLE.COM</value>
    </property>
    <property>
        <name>pxf.service.kerberos.keytab</name>
        <value>${pxf.conf}/keytabs/pxf.service.keytab</value>
    </property>
    ```
    
    PXF automatically replaces ` _HOST` with the FQDN of the host.

6. [Complete the PXF Configuration](#proc_complete) based on your chosen Hadoop access scenario.

### <a id="proc_complete"></a>Completing the PXF Configuration

On the Greenplum Database coordinator host, complete the configuration of the PXF server based on your chosen Hadoop access scenario. Choose one, as *these are mutually exclusive*:

1. If you want to access Hadoop as the Greenplum Database user:

    1. Activate user impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers) (this is the default setting).
    1. *If you want to use Kerberos constrained delegation*, [activate](#enable_kdc) it for the server, and configure AD or IPA to provide the PXF Kerberos principal with the delegation rights for the Greenplum end users.
    2. *If you did not activate Kerberos constrained delegation*, configure Hadoop proxying for the *primary* component of the Kerberos principal as described in [Configure Hadoop Proxying](pxfuserimpers.html#hadoop). For example, if your principal is `gpadmin/_HOST@EXAMPLE.COM`, configure proxying for the Hadoop user `gpadmin`.

1. If you want to access Hadoop using the identity of the Kerberos principal, deactivate user impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers).

1. If you want to access Hadoop as a custom user:

    1. Deactivate user impersonation as described in [Configure PXF User Impersonation](pxfuserimpers.html#pxf_cfg_impers).
    2. Configure the custom user name as described in [Configure the Hadoop User](pxfuserimpers.html#pxf_cfg_user).
    1. *If you want to use Kerberos constrained delegation*, [activate](#enable_kcd) it for the server, and configure AD or IPA to provide the PXF Kerberos principal with the delegation rights for the custom user.
    3. *If you did not activate Kerberos constrained delegation*, configure Hadoop proxying for the *primary* component of the Kerberos principal as described in [Configure Hadoop Proxying](pxfuserimpers.html#hadoop). For example, if your principal is `gpadmin/_HOST@EXAMPLE.COM`, configure proxying for the Hadoop user `gpadmin`.

1. Synchronize the PXF configuration to your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

### <a id="enable_kcd"></a>Activating Kerberos Constrained Delegation

By default, Kerberos constrained delegation is deactivated for PXF. Perform the following procedure to configure Kerberos constrained delegation for a PXF server:

1. Log in to your Greenplum Database coordinator host as the administrative user:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. Identify the name of the Hadoop PXF server configuration that you want to update.

1. Navigate to the server configuration directory. For example, if the server is named `hdp3`:

    ```shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/hdp3
    ```

1. If the server configuration does not yet include a `pxf-site.xml` file, copy the template file to the directory. For example:

    ``` shell
    gpadmin@coordinator$ cp <PXF_INSTALL_DIR>/templates/pxf-site.xml .
    ```

1. Open the `pxf-site.xml` file in the editor of your choice, locate the `pxf.service.kerberos-constrained.delegation` property, and set it as follows:

    ``` xml
    <property>
        <name>pxf.service.kerberos-constrained.delegation</name>
        <value>true</value>
    </property>
    ```

1. Save the `pxf-site.xml` file and exit the editor.

1. Use the `pxf cluster sync` command to synchronize the PXF Hadoop server configuration to your Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

