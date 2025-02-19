---
title: Monitoring PXF
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

You can monitor the status of PXF from the command line.

PXF also provides additional information about the runtime status of the PXF Service by exposing HTTP endpoints that you can use to query the health, build information, and various metrics of the running process.


## <a id="status"></a> Viewing PXF Status on the Command Line

The `pxf cluster status` command displays the status of the PXF Service instance on all hosts in your Greenplum Database cluster. `pxf status` displays the status of the PXF Service instance on the local Greenplum host.

Only the `gpadmin` user can request the status of the PXF Service.

Perform the following procedure to request the PXF status of your Greenplum Database cluster.

1. Log in to the Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Run the `pxf cluster status` command:

    ```shell
    gpadmin@coordinator$ pxf cluster status
    ```

## <a id="about_rtm"></a> About PXF Service Runtime Monitoring

PXF exposes the following HTTP endpoints that you can use to monitor a running PXF Service on the local host:

- `actuator/health` - Returns the status of the PXF Service.
- `actuator/info` - Returns build information for the PXF Service.
- `actuator/metrics` - Returns JVM, extended Tomcat, system, process, Log4j2, and PXF-specific metrics for the PXF Service.
- `actuator/prometheus` - Returns all metrics in a format that can be scraped by a Prometheus server.

Any user can access the HTTP endpoints and view the monitoring information that PXF returns.

You can view the data associated with a specific endpoint by viewing in a browser, or `curl`-ing, a URL of the following format (default PXF deployment topology):

``` pre
http://localhost:5888/<endpoint>[/<name>]
```

If you chose the [alternate deployment topology](deployment_topos.html#alt_topo) for PXF, the URL is:

``` pre
http://<pxf_listen_address>:<port>/<endpoint>[/<name>]
```

For example, to view the build information for the PXF service running on `localhost`, query the `actuator/info` endpoint:

``` pre
http://localhost:5888/actuator/info
```

Sample output:

``` pre
{"build":{"version":"6.0.0","artifact":"pxf-service","name":"pxf-service","pxfApiVersion":"16","group":"org.greenplum.pxf","time":"2021-03-29T22:26:22.780Z"}}
```

To view the status of the PXF Service running on the local Greenplum Database host, query the `actuator/health` endpoint:

``` pre
http://localhost:5888/actuator/health
```

Sample output:

``` pre
{"status":"UP","groups":["liveness","readiness"]}
```

### <a id="examine_metrics"></a> Examining PXF Metrics

PXF exposes JVM, extended Tomcat, and system metrics via its integration with Spring Boot. Refer to [Supported Metrics](https://docs.spring.io/spring-boot/docs/current/reference/html/production-ready-features.html#production-ready-metrics-meter) in the Spring Boot documentation for more information about these metrics.

PXF also exposes metrics that are specific to its processing, including:

| Metric Name  | Description |
|---------|-------------|
| pxf.fragments.sent  | The number of fragments, and the total time that it took to send all fragments to Greenplum Database. |
| pxf.records.sent  | The number of records that PXF sent to Greenplum Database. |
| pxf.records.received  | The number of records that PXF received from Greenplum Database. |
| pxf.bytes.sent  | The number of bytes that PXF sent to Greenplum Database. |
| pxf.bytes.received  | The number of bytes that PXF received from Greenplum Database. |
| http.server.requests | Standard metric augmented with PXF tags. |


The information that PXF returns when you query a metric is the aggregate data collected since the last (re)start of the PXF Service.

To view a list of all of the metrics (names) available from the PXF Service, query just the `metrics` endpoint:

``` pre
http://localhost:5888/actuator/metrics
```

### <a id="filter_metrics"></a> Filtering Metric Data

PXF tags all metrics that it returns with an `application` label; the value of this tag is always `pxf-service`.

PXF tags its specific metrics with the additional labels: `user`, `segment`, `profile`, and `server`. All of these tags are present for each PXF metric.  PXF returns the tag value `unknown` when the value cannot be determined.

You can use the tags to filter the information returned for PXF-specific metrics. For example, to examine the `pxf.records.received` metric for the PXF server named `hadoop1` located on `segment` 1 on the local host:

``` pre
http://localhost:5888/actuator/metrics/pxf.records.received?tag=segment:1&tag=server:hadoop1
```

Certain metrics, such as `pxf.fragments.sent`, include an additional tag named `outcome`; you can examine its value (`success` or `error`) to determine if all data for the fragment was sent. You can also use this tag to filter the aggregated data.

