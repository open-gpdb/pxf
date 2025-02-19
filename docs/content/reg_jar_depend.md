---
title: Registering Library Dependencies
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

You use PXF to access data stored on external systems. Depending upon the external data store, this access may require that you install and/or configure additional components or services for the external data store.

PXF depends on JAR files and other configuration information provided by these additional components. In most cases, PXF manages internal JAR dependencies as necessary based on the connectors that you use.

Should you need to register a JAR or native library dependency with PXF, you copy the library to a location known to PXF *or* you inform PXF of a custom location, and then you must synchronize and restart PXF.


## <a id="reg_jar"></a> Registering a JAR Dependency

PXF loads JAR dependencies from the following directories, in this order:

1. The directories that you specify in the `$PXF_BASE/conf/pxf-env.sh` configuration file, `PXF_LOADER_PATH` environment variable. The `pxf-env.sh` file includes this commented-out block:

    ``` pre
    # Additional locations to be class-loaded by PXF
    # export PXF_LOADER_PATH=
    ```

    You would uncomment the `PXF_LOADER_PATH` setting and specify one or more colon-separated directory names.

2. The default PXF JAR directory `$PXF_BASE/lib`.

To add a JAR dependency for PXF, for example a MySQL driver JAR file, you must log in to the Greenplum Database coordinator host, copy the JAR file to the PXF user configuration runtime library directory (`$PXF_BASE/lib`), sync the PXF configuration to the Greenplum Database cluster, and then restart PXF on each host. For example:

``` shell
$ ssh gpadmin@<coordinator>
gpadmin@coordinator$ cp new_dependent_jar.jar $PXF_BASE/lib/
gpadmin@coordinator$ pxf cluster sync
gpadmin@coordinator$ pxf cluster restart
```

Alternatively, you could have identified the file system location of the JAR in the `pxf-env.sh` `PXF_LOADER_PATH` environment variable. If you choose this registration option, you must ensure that you copy the JAR file to the same location on the Greenplum Database standby coordinator host and segment hosts before you synchronize and restart PXF.


## <a id="reg_native"></a> Registering a Native Library Dependency

PXF loads native libraries from the following directories, in this order:

1. The directories that you specify in the `$PXF_BASE/conf/pxf-env.sh` configuration file, `LD_LIBRARY_PATH` environment variable. The `pxf-env.sh` file includes this commented-out block:

    ``` pre
    # Additional native libraries to be loaded by PXF
    # export LD_LIBRARY_PATH=
    ```

    You would uncomment the `LD_LIBRARY_PATH` setting and specify one or more colon-separated directory names.

2. The default PXF native library directory `$PXF_BASE/lib/native`.
3. The default Hadoop native library directory `/usr/lib/hadoop/lib/native`.

As such, you have three file location options when you register a native library with PXF:

- Copy the library to the default PXF native library directory, `$PXF_BASE/lib/native`, on only the Greenplum Database coordinator host. When you next synchronize PXF, PXF copies the native library to all hosts in the Greenplum cluster.
- Copy the library to the default Hadoop native library directory, `/usr/lib/hadoop/lib/native`, on the Greenplum coordinator host, standby coordinator host, and each segment host.
- Copy the library to the same custom location on the Greenplum coordinator host, standby coordinator host, and each segment host, and uncomment and add the directory path to the `pxf-env.sh` `LD_LIBRARY_PATH` environment variable.


### <a id="reg_native_proc"></a> Procedure

1. Copy the native library file to one of the following:
    - The `$PXF_BASE/lib/native` directory on the Greenplum Database coordinator host. (You may need to create this directory.)
    - The `/usr/lib/hadoop/lib/native` directory on all Greenplum Database hosts.
    - A user-defined location on all Greenplum Database hosts; note the file system location of the native library.

2. If you copied the native library to a custom location:

    1. Open the `$PXF_BASE/conf/pxf-env.sh` file in the editor of your choice, and uncomment the `LD_LIBRARY_PATH` setting:

        ``` pre
        # Additional native libraries to be loaded by PXF
        export LD_LIBRARY_PATH=
        ```

    2. Specify the custom location in the `LD_LIBRARY_PATH` environment variable. For example, if you copied a library named `dependent_native_lib.so` to `/usr/local/lib` on all Greenplum hosts, you would set `LD_LIBRARY_PATH` as follows:

        ``` shell
        export LD_LIBRARY_PATH=/usr/local/lib
        ```

    3. Save the file and exit the editor.

3. Synchronize the PXF configuration from the Greenplum Database coordinator host to the standby coordinator host and segment hosts.

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

    If you copied the native library to the `$PXF_BASE/lib/native` directory, this command copies the library to the same location on the Greenplum Database standby coordinator host and segment hosts.

    If you updated the `pxf-env.sh` `LD_LIBRARY_PATH` environment variable, this command copies the configuration change to the Greenplum Database standby coordinator host and segment hosts.

4. Restart PXF on all Greenplum hosts:

    ``` shell
    gpadmin@coordinator$ pxf cluster restart
    ```

