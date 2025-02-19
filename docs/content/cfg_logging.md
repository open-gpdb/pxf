---
title: Logging
---

PXF provides two categories of message logging: service-level and client-level.

PXF manages its service-level logging, and supports the following log levels (more to less severe):

- fatal
- error
- warn
- info
- debug
- trace

The default configuration for the PXF Service logs at the `info` and more severe levels. For some third-party libraries, the PXF Service logs at the `warn` or `error` and more severe levels to reduce verbosity.

- PXF captures messages written to `stdout` and `stderr` and writes them to the `$PXF_LOGDIR/pxf-app.out` file. This file may contain service startup messages that PXF logs before logging is fully configured. The file may also contain debug output.
- Messages that PXF logs after start-up are written to the `$PXF_LOGDIR/pxf-service.log` file.

You can change the PXF log directory if you choose.

Client-level logging is managed by the Greenplum Database client; this topic details configuring logging for a `psql` client.

Enabling more verbose service-level or client-level logging for PXF may aid troubleshooting efforts.

## <a id="cfglogdir"></a>Configuring the Log Directory

The default PXF logging configuration writes log messages to `$PXF_LOGDIR`, where the default log directory is `PXF_LOGDIR=$PXF_BASE/logs`.

To change the PXF log directory, you must update the `$PXF_LOGDIR` property in the `pxf-env.sh` configuration file, synchronize the configuration change to the Greenplum Database cluster, and then restart PXF.

**Note:** The new log directory must exist on all Greenplum Database hosts, and must be accessible by the `gpadmin` user.

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. Use a text editor to uncomment the `export PXF_LOGDIR` line in `$PXF_BASE/conf/pxf-env.sh`, and replace the value with the new PXF log directory. For example:

    ``` xml
    # Path to Log directory
    export PXF_LOGDIR="/new/log/dir"
    ```

2. Use the `pxf cluster sync` command to copy the updated `pxf-env.sh` file to all hosts in the Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

3. Restart PXF on each Greenplum Database host as described in [Restarting PXF](cfginitstart_pxf.html#restart_pxf).

## <a id="pxfsvclogmsg"></a>Configuring Service-Level Logging

PXF utilizes Apache Log4j 2 for service-level logging. PXF Service-related log messages are captured in `$PXF_LOGDIR/pxf-app.out` and `$PXF_LOGDIR/pxf-service.log`. The default configuration for the PXF Service logs at the `info` and more severe levels.

You can change the log level for the PXF Service on a single Greenplum Database host, or on all hosts in the Greenplum cluster.

<div class="note"><b>Note:</b> PXF provides more detailed logging when the <code>debug</code> and <code>trace</code> log levels are enabled. Logging at these levels is quite verbose, and has both a performance and a storage impact. Be sure to turn it off after you have collected the desired information.</div>

### <a id="cfg_host"></a>Configuring for a Specific Host

You can change the log level for the PXF Service running on a specific Greenplum Database host in two ways:

- Setting the `PXF_LOG_LEVEL` environment variable on the `pxf restart` command line.
- Setting the log level via a property update.

**Procedure**:

1. Log in to the Greenplum Database host:

    ``` shell
    $ ssh gpadmin@<gphost>
    ```

1. Choose one of the following methods:
    - Set the log level on the `pxf restart` command line. For example, to change the log level from `info` (the default) to `debug`:

        ``` shell
        gpadmin@gphost$ PXF_LOG_LEVEL=debug pxf restart
        ```
    - Set the log level in the `pxf-application.properties` file:
        1. Use a text editor to uncomment the following line in the `$PXF_BASE/conf/pxf-application.properties` file and set the desired log level.  For example, to change the log level from `info` (the default) to `debug`:

            ``` xml
            pxf.log.level=debug
            ```
        1. Restart PXF on the host:

            ``` shell
            gpadmin@gphost$ pxf restart
            ```

4. `debug` logging is now enabled. Make note of the time; this will direct you to the relevant log messages in `$PXF_LOGDIR/pxf-service.log`.

    ``` shell
    $ date
    Wed Oct  4 09:30:06 MDT 2017
    $ psql -d <dbname>
    ```

1. Perform operations that exercise the PXF Service.

5. Collect and examine the log messages in `pxf-service.log`.

6. Depending upon how you originally set the log level, reinstate `info`-level logging on the host:
    - Command line method:

        ``` shell
        gpadmin@gphost$ pxf restart
        ```
    - Properties file method: Comment out the line or set the property value back to `info`, and then restart PXF on the host.


### <a id="cfg_cluster"></a>Configuring for the Cluster

To change the log level for the PXF service running on every host in the Greenplum Database cluster:

1. Log in to the Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

1. Use a text editor to uncomment the following line in the `$PXF_BASE/conf/pxf-application.properties` file and set the desired log level.  For example, to change the log level from `info` (the default) to `debug`:

    ``` xml
    pxf.log.level=debug
    ```

1. Use the `pxf cluster sync` command to copy the updated `pxf-application.properties` file to all hosts in the Greenplum Database cluster. For example:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

1.  Restart PXF on each Greenplum Database host:

    ``` shell
    gpadmin@coordinator$ pxf cluster restart
    ```

1. Perform operations that exercise the PXF Service, and then collect and examine the information in `$PXF_LOGDIR/pxf-service.log`.

1. Reinstate `info`-level logging by repeating the steps above with `pxf.log.level=info`.


## <a id="pxfdblogmsg"></a>Configuring Client-Level Logging

Database-level client session logging may provide insight into internal PXF Service operations.

Enable Greenplum Database client debug message logging by setting the `client_min_messages` server configuration parameter to `DEBUG2` in your `psql` session. This logging configuration writes messages to `stdout`, and will apply to all operations that you perform in the session, including operations on PXF external tables. For example:

``` shell
$ psql -d <dbname>
```

``` sql
dbname=# SET client_min_messages=DEBUG2;
dbname=# SELECT * FROM hdfstest;
...
DEBUG2:  churl http header: cell #26: X-GP-URL-HOST: localhost  (seg0 slice1 127.0.0.1:7002 pid=10659)
CONTEXT:  External table pxf_hdfs_textsimple, line 1 of file pxf://data/pxf_examples/pxf_hdfs_simple.txt?PROFILE=hdfs:text
DEBUG2:  churl http header: cell #27: X-GP-URL-PORT: 5888  (seg0 slice1 127.0.0.1:7002 pid=10659)
CONTEXT:  External table pxf_hdfs_textsimple, line 1 of file pxf://data/pxf_examples/pxf_hdfs_simple.txt?PROFILE=hdfs:text
DEBUG2:  churl http header: cell #28: X-GP-DATA-DIR: data%2Fpxf_examples%2Fpxf_hdfs_simple.txt  (seg0 slice1 127.0.0.1:7002 pid=10659)
CONTEXT:  External table pxf_hdfs_textsimple, line 1 of file pxf://data/pxf_examples/pxf_hdfs_simple.txt?PROFILE=hdfs:text
DEBUG2:  churl http header: cell #29: X-GP-TABLE-NAME: pxf_hdfs_textsimple  (seg0 slice1 127.0.0.1:7002 pid=10659)
CONTEXT:  External table pxf_hdfs_textsimple, line 1 of file pxf://data/pxf_examples/pxf_hdfs_simple.txt?PROFILE=hdfs:text
...
```

Collect and examine the log messages written to `stdout`.

**Note**: `DEBUG2` database client session logging has a performance impact.  Remember to turn off `DEBUG2` logging after you have collected the desired information.

``` sql
dbname=# SET client_min_messages=NOTICE;
```

