---
title: About the Configuration Files
---

`$PXF_BASE/conf` includes these user-customizable configuration files:

- `pxf-application.properties` - PXF Service application configuration properties
- `pxf-env.sh` - PXF command and JVM-specific runtime configuration properties
- `pxf-log4j2.xml` - PXF logging configuration properties
- `pxf-profiles.xml` - Custom PXF profile definitions


## <a id="chg_config"></a>Modifying the PXF Configuration

When you update a PXF configuration file, you must synchronize the changes to all hosts in the Greenplum Database cluster and then restart PXF for the changes to take effect.

Procedure:

1. Update the configuration file(s) of interest.

1. Synchronize the PXF configuration to all hosts in the Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

1. (Re)start PXF on all Greenplum hosts:

    ``` shell
    gpadmin@coordinator$ pxf cluster restart
    ```


## <a id="pxfappprops"></a>pxf-application.properties

The `pxf-application.properties` file exposes these PXF Service application configuration properties:

| Parameter  | Description | Default Value |
|-----------|---------------| ------------|
| pxf.connection.timeout | The Tomcat server connection timeout for read operations (-1 for infinite timeout). | 5m (5 minutes) |
| pxf.connection.upload-timeout | The Tomcat server connection timeout for write operations (-1 for infinite timeout). | 5m (5 minutes) |
| [pxf.max.threads](cfg_mem.html#pxf-threadcfg) | The maximum number of PXF tomcat threads. | 200 |
| pxf.task.pool.allow&#8209;core&#8209;thread&#8209;timeout | Identifies whether or not core streaming threads are allowed to time out. | false |
| pxf.task.pool.core-size | The number of core streaming threads. | 8 |
| pxf.task.pool.queue-capacity | The capacity of the core streaming thread pool queue. | 0 |
| pxf.task.pool.max-size | The maximum allowed number of core streaming threads. | pxf.max.threads if set, or 200 |
| [pxf.log.level](cfg_logging.html) | The log level for the PXF Service. | info  |
| pxf.fragmenter-cache.expiration | The amount of time after which an entry expires and is removed from the fragment cache. | 10s (10 seconds) |
| [server.address](cfghostport.html) | The PXF server listen address. | localhost |

To change the value of a PXF Service application property, you may first need to add the property to, or uncomment the property in, the `pxf-application.properties` file before you can set the new value.


## <a id="pxfenvsh"></a>pxf-env.sh

The `pxf-env.sh` file exposes these PXF JVM configuration properties:

| Parameter  | Description | Default Value |
|-----------|---------------| ------------|
| [JAVA_HOME](instcfg_pxf.html)    | The path to the Java JRE home directory. | /usr/java/default |
| [PXF_LOGDIR](cfg_logging.html#cfglogdir)   | The PXF log directory. | $PXF_BASE/logs |
| PXF_RUNDIR   | The PXF run directory. | $PXF_BASE/run |
| [PXF_JVM_OPTS](cfg_mem.html#pxf-heapcfg)  | The default options for the PXF Java virtual machine. | -Xmx2g -Xms1g |
| [PXF_OOM_KILL](cfg_mem.html#pxf-cfgoom)  | Activate/deactivate PXF auto-termination on OutOfMemoryError (OOM). | true (activated) |
| [PXF_OOM_DUMP_PATH](cfg_mem.html#pxf-cfgoom)  | The absolute path to the dump file that PXF generates on OOM. | No dump file (empty) |
| [PXF_LOADER_PATH](reg_jar_depend.html)  | Additional directories and JARs for PXF to class-load. | (empty) |
| [LD_LIBRARY_PATH](reg_jar_depend.html)  | Additional directories and native libraries for PXF to load. | (empty) |

To set a new value for a PXF JVM configuration property, you may first need to uncomment the property in the `pxf-env.sh` file before you set the new value.


## <a id="pxflog4j2"></a>pxf-log4j2.xml

The `pxf-log4j2.xml` file configures PXF and subcomponent logging. By default, PXF is configured to log at the `info` level, and logs at the `warn` or `error` levels for some third-party libraries to reduce verbosity.

The [Logging](cfg_logging.html) advanced configuration topic describes how to enable more verbose client-level and server-level logging for PXF.

## <a id="pxfprofiledef"></a>pxf-profiles.xml

PXF defines its default profiles in the [`pxf-profiles-default.xml`](https://github.com/greenplum-db/pxf/blob/main/server/pxf-service/src/main/resources/pxf-profiles-default.xml) file. If you choose to add a custom profile, you configure the profile in `pxf-profiles.xml`.

