---
title: Memory and Threading
---

Because a single PXF Service (JVM) serves multiple segments on a segment host, the PXF heap size can be a limiting runtime factor. This becomes more evident under concurrent workloads or with queries against large files. You may run into situations where a query hangs or fails due to insufficient memory or the Java garbage collector impacting response times. To avert or remedy these situations, first try increasing the Java maximum heap size or decreasing the Tomcat maximum number of threads, depending upon what works best for your system configuration. You may also choose to configure PXF to [auto-terminate the server](#pxf-cfgoom-autoterm) (activated by default) or [dump the Java heap](#pxf-cfgoom-heapdump) when it detects an out of memory condition.


## <a id="pxf-heapcfg"></a>Increasing the JVM Memory for PXF

Each PXF Service running on a Greenplum Database host is configured with a default maximum Java heap size of 2GB and an initial heap size of 1GB. If the hosts in your Greenplum Database cluster have an ample amount of memory, try increasing the maximum heap size to a value between 3-4GB. Set the initial and maximum heap size to the same value if possible.

Perform the following procedure to increase the heap size for the PXF Service running on each host in your Greenplum Database cluster.

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Edit the `$PXF_BASE/conf/pxf-env.sh` file. For example:

    ``` shell
    gpadmin@coordinator$ vi $PXF_BASE/conf/pxf-env.sh
    ```

3. Locate the `PXF_JVM_OPTS` setting in the `pxf-env.sh` file, and update the `-Xmx` and/or `-Xms` options to the desired value. For example:

    ``` shell
    PXF_JVM_OPTS="-Xmx3g -Xms3g"
    ```

3. Save the file and exit the editor.

4. Use the `pxf cluster sync` command to copy the updated `pxf-env.sh` file to the Greenplum Database cluster. For example:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

5. Restart PXF on each Greenplum Database host as described in [Restarting PXF](cfginitstart_pxf.html#restart_pxf).


## <a id="pxf-cfgoom"></a>Configuring Out of Memory Condition Actions

In an out of memory (OOM) situation, PXF returns the following error in response to a query:

``` pre
java.lang.OutOfMemoryError: Java heap space
```

You can configure the PXF JVM to activate/deactivate the following actions when it detects an OOM condition:

- Auto-terminate the PXF Service (activated by default).
- Dump the Java heap (deactivated by default).

### <a id="pxf-cfgoom-autoterm"></a>Auto-Terminating the PXF Server

By default, PXF is configured such that when the PXF JVM detects an out of memory condition on a Greenplum host, it automatically runs a script that terminates the PXF Service running on the host. The `PXF_OOM_KILL` environment variable in the `$PXF_BASE/conf/pxf-env.sh` configuration file governs this auto-terminate behavior.

When auto-terminate is activated and the PXF JVM detects an OOM condition and terminates the PXF Service on the host:

- PXF logs the following messages to `$PXF_LOGDIR/pxf-oom.log` on the segment host:

    ``` shell
    =====> <date> PXF Out of memory detected <======
    =====> <date> PXF shutdown scheduled <======
    =====> <date> Stopping PXF <======
    ```

- Any query that you run on a PXF external table will fail with the following error until you restart the PXF Service on the host:

    ``` shell
    ... Failed to connect to <host> port 5888: Connection refused
    ```

**When the PXF Service on a host is shut down in this manner, you must explicitly restart the PXF Service on the host.** See the [pxf](ref/pxf.html) reference page for more information on the `pxf start` command.

Refer to the configuration [procedure](#pxf-cfgoom_proc) below for the instructions to deactivate/activate this PXF configuration property.
 
### <a id="pxf-cfgoom-heapdump"></a>Dumping the Java Heap

In an out of memory situation, it may be useful to capture the Java heap dump to help determine what factors contributed to the resource exhaustion. You can configure PXF to write the heap dump to a file when it detects an OOM condition by setting the `PXF_OOM_DUMP_PATH` environment variable in the `$PXF_BASE/conf/pxf-env.sh` configuration file. By default, PXF does not dump the Java heap on OOM.

If you choose to activate the heap dump on OOM, you must set `PXF_OOM_DUMP_PATH` to the absolute path to a file or directory:

- If you specify a directory, the PXF JVM writes the heap dump to the file `<directory>/java_pid<pid>.hprof`, where `<pid>` identifies the process ID of the PXF Service instance. The PXF JVM writes a new file to the directory every time the JVM goes OOM.
- If you specify a file and the file does not exist, the PXF JVM writes the heap dump to the file when it detects an OOM. If the file already exists, the JVM will not dump the heap.

Ensure that the `gpadmin` user has write access to the dump file or directory.

**Note**: Heap dump files are often rather large. If you activate heap dump on OOM for PXF and specify a directory for `PXF_OOM_DUMP_PATH`, multiple OOMs will generate multiple files in the directory and could potentially consume a large amount of disk space. If you specify a file for `PXF_OOM_DUMP_PATH`, disk usage is constant when the file name does not change. You must rename the dump file or configure a different `PXF_OOM_DUMP_PATH` to generate subsequent heap dumps.

Refer to the configuration [procedure](#pxf-cfgoom_proc) below for the instructions to activate/deactivate this PXF configuration property.

#### <a id="pxf-cfgoom_proc"></a>Procedure

Auto-termination of the PXF Service on OOM is deactivated by default. Heap dump generation on OOM is deactivated by default. To configure one or both of these properties, perform the following procedure:

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Edit the `$PXF_BASE/conf/pxf-env.sh` file. For example:

    ``` shell
    gpadmin@coordinator$ vi $PXF_BASE/conf/pxf-env.sh
    ```

3. If you want to configure (i.e. turn off, or turn back on) auto-termination of the PXF Service on OOM, locate the `PXF_OOM_KILL` property in the `pxf-env.sh` file. If the setting is commented out, uncomment it, and then update the value. For example, to turn off this behavior, set the value to `false`:

    ``` shell
    export PXF_OOM_KILL=false
    ```

4. If you want to configure (i.e. turn on, or turn back off) automatic heap dumping when the PXF Service hits an OOM condition, locate the `PXF_OOM_DUMP_PATH` setting in the `pxf-env.sh` file.

    1. To turn this behavior on, set the `PXF_OOM_DUMP_PATH` property value to the file system location to which you want the PXF JVM to dump the Java heap. For example, to dump to a file named `/home/gpadmin/pxfoom_segh1`:

        ``` shell
        export PXF_OOM_DUMP_PATH=/home/pxfoom_segh1
        ```

    2. To turn off heap dumping after you have turned it on, comment out the `PXF_OOM_DUMP_PATH` property setting:

        ``` shell
        #export PXF_OOM_DUMP_PATH=/home/pxfoom_segh1
        ```

5. Save the `pxf-env.sh` file and exit the editor.

6. Use the `pxf cluster sync` command to copy the updated `pxf-env.sh` file to the Greenplum Database cluster. For example:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

7. Restart PXF on each Greenplum Database host as described in [Restarting PXF](cfginitstart_pxf.html#restart_pxf).


## <a id="pxf-threadcfg"></a>Another Option for Resource-Constrained PXF Segment Hosts

If increasing the maximum heap size is not suitable for your Greenplum Database deployment, try decreasing the number of concurrent working threads configured for PXF's embedded Tomcat web server. A decrease in the number of running threads will prevent any PXF server from exhausting its memory, while ensuring that current queries run to completion (albeit a bit slower). Tomcat's default behavior is to queue requests until a thread is free, or the queue is exhausted.

The default maximum number of Tomcat threads for PXF is 200. The `pxf.max.threads` property in the `pxf-application.properties` configuration file controls this setting.

If you plan to run large workloads on a large number of files in an external Hive data store, or you are reading compressed ORC or Parquet data, consider specifying a lower `pxf.max.threads` value. Large workloads require more memory, and a lower thread count limits concurrency, and hence, memory consumption.

**Note**: Keep in mind that an increase in the thread count correlates with an increase in memory consumption.

Perform the following procedure to set the maximum number of Tomcat threads for the PXF Service running on each host in your Greenplum Database deployment.

1. Log in to your Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Edit the `$PXF_BASE/conf/pxf-application.properties` file. For example:

    ``` shell
    gpadmin@coordinator$ vi $PXF_BASE/conf/pxf-application.properties
    ```

3. Locate the `pxf.max.threads` setting in the `pxf-application.properties` file. If the setting is commented out, uncomment it, and then update to the desired value. For example, to reduce the maximum number of Tomcat threads to 100:

    ``` shell
    pxf.max.threads=100
    ```

3. Save the file and exit the editor.

4. Use the `pxf cluster sync` command to copy the updated `pxf-application.properties` file to the Greenplum Database cluster. For example:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

5. Restart PXF on each Greenplum Database host as described in [Restarting PXF](cfginitstart_pxf.html#restart_pxf).

