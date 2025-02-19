---
title: About PXF Filter Pushdown
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

PXF supports filter pushdown. When filter pushdown is activated, the constraints from the `WHERE` clause of a `SELECT` query can be extracted and passed to the external data source for filtering. This process can improve query performance, and can also reduce the amount of data that is transferred to Greenplum Database.

You activate or deactivate filter pushdown for all external table protocols, including `pxf`, by setting the `gp_external_enable_filter_pushdown` server configuration parameter. The default value of this configuration parameter is `on`; set it to `off` to deactivate filter pushdown. For example:

``` sql
SHOW gp_external_enable_filter_pushdown;
SET gp_external_enable_filter_pushdown TO 'on';
```

**Note:** Some external data sources do not support filter pushdown. Also, filter pushdown may not be supported with certain data types or operators. If a query accesses a data source that does not support filter push-down for the query constraints, the query is instead run without filter pushdown (the data is filtered after it is transferred to Greenplum Database).

PXF filter pushdown can be used with these data types (connector- and profile-specific):

- `INT2`, `INT4`, `INT8`
- `CHAR`, `TEXT`, `VARCHAR`
- `FLOAT`
- `NUMERIC` (not available with the `hive` profile when accessing `STORED AS Parquet`)
- `BOOL`
- `DATE`, `TIMESTAMP` (available only with the JDBC connector, the S3 connector when using S3 Select, the `hive:rc` and `hive:orc` profiles, and the `hive` profile when accessing `STORED AS` `RCFile` or `ORC`)

PXF accesses data sources using profiles exposed by different connectors, and filter pushdown support is determined by the specific connector implementation. 
The following PXF profiles support some aspects of filter pushdown as well as different arithmetic and logical operations:


| Profile | <, >, <=, >=, =, <> | LIKE  | IS [NOT] NULL | IN | AND | OR | NOT |
| ------- | :-----------------: | :----: | :----: | :----: | :----: | :----: | :----: |
| jdbc | Y | Y^4^ | Y | N | Y | Y | Y |
| *:parquet | Y^1^ | N | Y^1^ | Y^1^ | Y^1^ | Y^1^ | Y^1^ |
| *:orc (all except hive:orc) | Y^1,3^ | N | Y^1,3^ | Y^1,3^ | Y^1,3^ | Y^1,3^ | Y^1,3^ |
| s3:parquet and s3:text with S3-Select | Y |  N | Y | Y | Y | Y | Y |
| hbase | Y | N | Y | N | Y | Y | N |
| hive:text | Y^2^ | N | N | N | Y^2^ | Y^2^ | N |
| hive:rc, hive (accessing stored as RCFile) | Y^2^ |  N | Y | Y | Y, Y^2^ | Y, Y^2^ | Y |
| hive:orc, hive (accessing stored as ORC) | Y, Y^2^ |  N | Y | Y | Y, Y^2^ | Y, Y^2^ | Y |
| hive (accessing stored as Parquet) | Y, Y^2^ | N | N | Y | Y, Y^2^ | Y, Y^2^ | Y |
| hive:orc and VECTORIZE=true | Y^2^ |  N | N | N | Y^2^ | Y^2^ | N |

</br><sup>1</sup>&nbsp;PXF applies the predicate, rather than the remote system, reducing CPU usage and the memory footprint.
</br><sup>2</sup>&nbsp;PXF supports partition pruning based on partition keys.
</br><sup>3</sup>&nbsp;PXF filtering is based on file-level, stripe-level, and row-level ORC statistics.
</br><sup>4</sup>&nbsp;The PXF `jdbc` profile supports the `LIKE` operator only for `TEXT` fields.

PXF does not support filter pushdown for any profile not mentioned in the table above, including: *:avro, *:AvroSequenceFile, *:SequenceFile, *:json, *:text, *:csv, `*:fixedwidth`, and *:text:multi.

To summarize, all of the following criteria must be met for filter pushdown to occur:

* You activate external table filter pushdown by setting the `gp_external_enable_filter_pushdown` server configuration parameter to `'on'`.
* The Greenplum Database protocol that you use to access external data source must support filter pushdown. The `pxf` external table protocol supports pushdown.
* The external data source that you are accessing must support pushdown. For example, HBase and Hive support pushdown.
* For queries on external tables that you create with the `pxf` protocol, the underlying PXF connector must also support filter pushdown. For example, the PXF Hive, HBase, and JDBC connectors support pushdown, as do the PXF connectors that support reading ORC and Parquet data.

    Refer to Hive [Partition Pruning](hive_pxf.html#partitionfiltering) for more information about Hive support for this feature.

