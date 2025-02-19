---
title: Greenplum® Platform Extension Framework (PXF)
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

The Greenplum Platform Extension Framework (PXF) provides parallel, high throughput data access and federated queries across heterogeneous data sources via built-in connectors that map a Greenplum Database external table definition to an external data source. PXF has its roots in the Apache HAWQ project.

-   [Overview of PXF](content/overview_pxf.html)

-   [Introduction to PXF](content/intro_pxf.html)

    This topic introduces PXF concepts and usage.

-   [Administering PXF](content/about_pxf_dir.html)

    This set of topics details the administration of PXF including configuration and management procedures.

-   [Accessing Hadoop with PXF](content/access_hdfs.html)

    This set of topics describe the PXF Hadoop connectors, the data types they support, and the profiles that you can use to read from and write to HDFS.

-   [Accessing Azure, Google Cloud Storage, and S3-Compatible Object Stores with PXF](content/access_objstore.html)

    This set of topics describe the PXF object storage connectors, the data types they support, and the profiles that you can use to read data from and write data to the object stores.

-   [Accessing an SQL Database with PXF (JDBC)](content/jdbc_pxf.html)

    This topic describes how to use the PXF JDBC connector to read from and write to an external SQL database such as Postgres or MySQL.

-   [Accessing Files on a Network File System with PXF](content/nfs_pxf.html)

    This topic describes how to use PXF to access files on a network file system that is mounted on your Greenplum Database hosts.

-   [Troubleshooting PXF](content/troubleshooting_pxf.html)

    This topic details the service-level and database-level logging configuration procedures for PXF. It also identifies some common PXF errors and describes how to address PXF memory issues.

-   [PXF Utility Reference](content/ref/pxf-ref.html)

    The PXF utility reference.

