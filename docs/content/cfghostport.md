---
title: Service Listen Address, Host, and Port
---

In the default deployment topology, since PXF 6.7.0, the PXF Service starts on a Greenplum host and listens on `localhost:5888`. With this configuration, the PXF Service listens for local traffic on the Greenplum host. You can configure PXF to listen on a different listen address. You can also configure PXF to listen on a different port number, or to run on a different host. To change the default configuration, you set one or more of the properties identified below:

| Property | Type | Description | Default |
| --- | --- | --- | --- |
|   server.address  | `pxf-application.properties` property | The PXF server listen address. | `localhost`  |
|   PXF_HOST  | Environment variable | The name or IP address of the (non-Greenpum) host on which the PXF Service is running. | `localhost`  |
|   PXF_PORT  | Environment variable | The port number on which the PXF server listens for requests on the host. | `5888`  |


## <a id="listen_address"></a>Configuring the Listen Address

The `server.address` property identifies the IP address or hostname of the network interface on which the PXF service listens. The default PXF service listen address is `localhost`. You may choose to change the listen address to allow traffic from other hosts to send requests to PXF (for example, when you have chosen the [alternate deployment topology](deployment_topos.html#alt_topo) or to retrieve PXF monitoring data).

Perform the following procedure to change the PXF listen address:

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. Locate the `pxf-application.properties` file in your PXF installation. If you did not relocate `$PXF_BASE`, the file resides here:

    ``` pre
    /usr/local/pxf-gp6/conf/pxf-application.properties
    ```

1. Open the file in the editor of your choice,  uncomment and set the following line:

    ``` pre
    server.address=<new_listen_addr>
    ```

    Changing the listen address to `0.0.0.0` allows PXF to listen for requests from all hosts.

1. Save the file and exit the editor.

1. Synchronize the PXF configuration and then restart PXF:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    gpadmin@coordinator$ pxf cluster restart
    ```

## <a id="port"></a>Configuring the Port Number

<div class="note"><b>Note:</b> You must restart both Greenplum Database and PXF when you configure the service port number in this manner. Consider performing this configuration during a scheduled down time.</div>

Perform the following procedure to configure the port number of the PXF server on one or more Greenplum Database hosts:

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. For each Greenplum Database host:

    1. Identify the port number on which you want the PXF Service to listen.
    1. Log in to the Greenplum Database host:

        ``` shell
        $ ssh gpadmin@<seghost>
        ```
    1. Open the `~/.bashrc` file in the editor of your choice.
    1. Set the `PXF_PORT` environment variable. For example, to set the PXF Service port number to 5998, add the following to the `.bashrc` file:

        ``` shell
        export PXF_PORT=5998
        ```
    1. Save the file and exit the editor.

1. Source the `.bashrc` that file you just updated:

    ``` shell
    gpadmin@coordinator$ source ~/.bashrc
    ```

3. Restart Greenplum Database as described in [Restarting Greenplum Database](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/admin_guide-managing-startstop.html#restarting-greenplum-database) in the Greenplum Documentation.

4. Restart PXF on each Greenplum Database host:

    ``` shell
    gpadmin@coordinator$ pxf cluster restart
    ```

5. Verify that PXF is running on the reconfigured port by invoking `http://<PXF_HOST>:<PXF_PORT>/actuator/health` to view PXF monitoring information as described in [About PXF Service Runtime Monitoring](monitor_pxf.html#about_rtm).


## <a id="host"></a>Configuring the Host

If you have chosen the [alternate deployment topology](deployment_topos.html#alt_topo) for PXF, you must set the `PXF_HOST` environment variable on each Greenplum segment host to inform Greenplum of the location of the PXF service. You must also set the listen address as described in [Configuring the Listen Address](#listen_address).

Perform the following procedure to configure the PXF host on each Greenplum Database segment host:

<div class="note"><b>Note:</b> You must restart Greenplum Database when you configure the host in this manner. Consider performing this configuration during a scheduled down time.</div>

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. For each Greenplum Database segment host:

    1. Identify the host name or IP address of a PXF Server.
    3. Log in to the Greenplum Database segment host:

        ``` shell
        $ ssh gpadmin@<seghost>
        ```
    4. Open the `~/.bashrc` file in the editor of your choice.
    5. Set the `PXF_HOST` environment variable. For example, to set the PXF host to `pxfalthost1`, add the following to the `.bashrc` file:

        ``` shell
        export PXF_HOST=pxfalthost1
        ```
    4. Save the file and exit the editor.

1. Source the `.bashrc` that file you just updated:

    ``` shell
    gpadmin@coordinator$ source ~/.bashrc
    ```

1. Configure the listen address of the PXF Service as described in [Configuring the Listen Address](#listen_address).

3. Restart Greenplum Database as described in [Restarting Greenplum Database](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/admin_guide-managing-startstop.html#restarting-greenplum-database) in the Greenplum Documentation.

5. Verify that PXF is running on the reconfigured host by invoking `http://<PXF_HOST>:<PXF_PORT>/actuator/health` to view PXF monitoring information as described in [About PXF Service Runtime Monitoring](monitor_pxf.html#about_rtm).

