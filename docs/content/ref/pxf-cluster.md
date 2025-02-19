---
title: pxf cluster
---

Manage the PXF configuration and the PXF Service instance on all Greenplum Database hosts.

## <a id="topic1__section2"></a>Synopsis

``` pre
pxf cluster <command> [<option>]
```

where `<command>` is:

``` pre
help
init (deprecated)
migrate
prepare
register
reset (deprecated)
restart
start
status
stop
sync
```

## <a id="topic1__section3"></a>Description

The `pxf cluster` utility command manages PXF on the coordinator host, standby coordinator host, and on all Greenplum Database segment hosts. You can use the utility to:

- Start, stop, and restart the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.
- Display the status of the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.
- Synchronize the PXF configuration from the Greenplum Database coordinator host to the standby coordinator and to all segment hosts.
- Copy the PXF extension control file from the PXF installation on each host to the Greenplum installation on the host after a Greenplum upgrade.
- Prepare a new `$PXF_BASE` runtime configuration directory.
- Migrate PXF 5 `$PXF_CONF` configuration to `$PXF_BASE`.

`pxf cluster` requires a running Greenplum Database cluster. You must run the utility on the Greenplum Database coordinator host.

If you want to manage the PXF Service instance on a specific segment host, use the `pxf` utility. See [`pxf`](pxf.html#topic1).

## <a id="commands"></a>Commands

<dt>help</dt>
<dd>Display the <code>pxf cluster</code> help message and then exit.</dd>

<dt>init (deprecated)</dt>
<dd>The command is equivalent to the <code>register</code> command.</dd>

<dt>migrate</dt>
<dd>Migrate the configuration in a PXF 5 <code>$PXF_CONF</code> directory to <code>$PXF_BASE</code> on each Greenplum Database host. When you run the command, you must identify the PXF 5 configuration directory via an environment variable named <code>PXF_CONF</code>. PXF migrates the version 5 configuration to <code>$PXF_BASE</code>, copying and merging files and directories as necessary. <div class="note"><b>Note:</b> You must manually migrate any <code>pxf-log4j.properties</code> customizations to the <code>pxf-log4j2.xml</code> file.</div></dd>

<dt>prepare</dt>
<dd>Prepare a new <code>$PXF_BASE</code> directory on each Greenplum Database host. When you run the command, you must identify the new PXF runtime configuration directory via an environment variable named <code>PXF_BASE</code>. PXF copies runtime configuration file templates and directories to this <code>$PXF_BASE</code>.</dd>

<dt>register</dt>
<dd>Copy the PXF extension control file from the PXF installation on each host to the Greenplum installation on the host. This command requires that <code>$GPHOME</code> be set, and is run once after you install PXF 6.x the first time, or run after you upgrade your Greenplum Database installation.</dd>

<dt>reset (deprecated) </dt>
<dd>The command is a no-op.</dd>

<dt>restart</dt>
<dd>Stop, and then start, the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.</dd>

<dt>start</dt>
<dd>Start the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.</dd>

<dt>status  </dt>
<dd>Display the status of the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.</dd>

<dt>stop  </dt>
<dd>Stop the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts.</dd>

<dt>sync  </dt>
<dd>Synchronize the PXF configuration (<code>$PXF_BASE</code>) from the coordinator host to the standby coordinator host and to all Greenplum Database segment hosts. By default, this command updates files on and copies files to the remote. You can instruct PXF to also delete files during the synchronization; see Options below.</dd>
<dd>If you have updated the PXF user configuration or add new JAR or native library dependencies, you must also restart PXF after you synchronize the PXF configuration.</dd>

## <a id="options"></a>Options

The `pxf cluster sync` command takes the following option:

<dt>&#8211;d | &#8211;&#8211;delete </dt>
<dd>Delete any files in the PXF user configuration on the standby coordinator host and segment hosts that are not also present on the coordinator host.</dd>

## <a id="topic1__section5"></a>Examples

Stop the PXF Service instance on the coordinator host, standby coordinator host, and all segment hosts:

``` shell
$ pxf cluster stop
```

Synchronize the PXF configuration to the standby coordinator host and all segment hosts, deleting files that do not exist on the coordinator host:

``` shell
$ pxf cluster sync --delete
```

## <a id="topic1__section6"></a>See Also

[`pxf`](pxf.html#topic1)
