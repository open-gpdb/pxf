package org.greenplum.pxf.automation.features.iceberg;

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

import org.greenplum.pxf.automation.features.BaseFeature;
import org.testng.annotations.Test;

import java.net.URI;
import java.net.HttpURLConnection;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

/**
 * End-to-end tests of the iceberg profile against an Iceberg REST catalog.
 * <p>
 * The tests expect:
 * <ul>
 *     <li>an Iceberg REST catalog reachable at {@code ICEBERG_REST_URI} (default {@code http://iceberg-rest:8181})
 *     whose warehouse is a directory that both the catalog and PXF can write to;</li>
 *     <li>a PXF server named {@code iceberg_rest} configured for that catalog
 *     (see server/pxf-iceberg/README.md).</li>
 * </ul>
 * Iceberg tables are (re)created through the REST API before the tests; Cloudberry external and foreign
 * tables are created here, while the data is written and read by the SQL test files.
 */
public class IcebergRestTest extends BaseFeature {

    private static final String REST_URI = System.getenv().getOrDefault("ICEBERG_REST_URI", "http://iceberg-rest:8181");
    private static final String NAMESPACE = "test";
    private static final String PXF_SERVER = "iceberg_rest";

    private static final String CUSTOMER_FIELDS = "id int, first_name text, last_name text, age int";
    private static final String TYPES_FIELDS = "id int, b boolean, l bigint, f real, d float8, dec numeric, dt date, " +
            "ts timestamp, tstz timestamptz, s text, t time, u uuid";
    private static final String EVENTS_FIELDS = "id int, region text, amount float8";

    private static final String CUSTOMER_SCHEMA = "{\"type\":\"struct\",\"schema-id\":0,\"fields\":[" +
            field(1, "id", "int") + "," +
            field(2, "first_name", "string") + "," +
            field(3, "last_name", "string") + "," +
            field(4, "age", "int") + "]}";
    private static final String TYPES_SCHEMA = "{\"type\":\"struct\",\"schema-id\":0,\"fields\":[" +
            field(1, "id", "int") + "," +
            field(2, "b", "boolean") + "," +
            field(3, "l", "long") + "," +
            field(4, "f", "float") + "," +
            field(5, "d", "double") + "," +
            field(6, "dec", "decimal(10,2)") + "," +
            field(7, "dt", "date") + "," +
            field(8, "ts", "timestamp") + "," +
            field(9, "tstz", "timestamptz") + "," +
            field(10, "s", "string") + "," +
            field(11, "t", "time") + "," +
            field(12, "u", "uuid") + "]}";
    private static final String EVENTS_SCHEMA = "{\"type\":\"struct\",\"schema-id\":0,\"fields\":[" +
            field(1, "id", "int") + "," +
            field(2, "region", "string") + "," +
            field(3, "amount", "double") + "]}";
    private static final String EVENTS_PARTITION_SPEC = "{\"spec-id\":0,\"fields\":[" +
            "{\"source-id\":2,\"field-id\":1000,\"name\":\"region\",\"transform\":\"identity\"}]}";


    @Override
    protected void beforeClass() throws Exception {
        rest("POST", "/v1/namespaces", "{\"namespace\":[\"" + NAMESPACE + "\"]}", 200, 409);
    }

    /**
     * Writes rows through a writable external table, reads them back, appends a second batch and
     * checks filter pushdown with IN, BETWEEN, AND and OR.
     */
    @Test(groups = {"iceberg"})
    public void writeAndReadExternalTable() throws Exception {
        recreateIcebergTable("customer", CUSTOMER_SCHEMA, null);
        createExternalTables("customer", CUSTOMER_FIELDS);

        runSqlTest("features/iceberg/write_read");
    }

    /**
     * Writes and reads all supported primitive types, including NULLs and a numeric value whose scale
     * differs from the scale of the Iceberg decimal column, and checks pushdown of typed filters.
     */
    @Test(groups = {"iceberg"})
    public void primitiveTypes() throws Exception {
        recreateIcebergTable("types", TYPES_SCHEMA, null);
        createExternalTables("types", TYPES_FIELDS);

        runSqlTest("features/iceberg/types");
    }

    /**
     * Writes to and reads from a table partitioned by an identity transform.
     */
    @Test(groups = {"iceberg"})
    public void identityPartitionedTable() throws Exception {
        recreateIcebergTable("events", EVENTS_SCHEMA, EVENTS_PARTITION_SPEC);
        createExternalTables("events", EVENTS_FIELDS);

        runSqlTest("features/iceberg/partitioned");
    }

    /**
     * Reads from and writes to Iceberg tables through pxf_fdw foreign tables.
     */
    @Test(groups = {"iceberg-fdw"})
    public void foreignTables() throws Exception {
        gpdb.runQuery("CREATE EXTENSION IF NOT EXISTS pxf_fdw", true, false);
        recreateIcebergTable("fdw_customer", CUSTOMER_SCHEMA, null);
        recreateIcebergTable("fdw_types", TYPES_SCHEMA, null);

        dropIfExists("DROP FOREIGN DATA WRAPPER IF EXISTS iceberg_pxf_fdw CASCADE");
        gpdb.runQuery("CREATE FOREIGN DATA WRAPPER iceberg_pxf_fdw HANDLER pxf_fdw_handler VALIDATOR pxf_fdw_validator " +
                "OPTIONS (protocol 'iceberg', mpp_execute 'all segments')");
        gpdb.runQuery("CREATE SERVER iceberg_rest_server FOREIGN DATA WRAPPER iceberg_pxf_fdw " +
                "OPTIONS (catalog_type 'REST', uri '" + REST_URI + "')");
        gpdb.runQuery("CREATE USER MAPPING FOR CURRENT_USER SERVER iceberg_rest_server");
        gpdb.runQuery("CREATE FOREIGN TABLE pxf_iceberg_fdw_customer (" + CUSTOMER_FIELDS + ") " +
                "SERVER iceberg_rest_server OPTIONS (resource '" + NAMESPACE + ".fdw_customer')");
        gpdb.runQuery("CREATE FOREIGN TABLE pxf_iceberg_fdw_types (" + TYPES_FIELDS + ") " +
                "SERVER iceberg_rest_server OPTIONS (resource '" + NAMESPACE + ".fdw_types')");

        runSqlTest("features/iceberg/fdw");
    }

    private void createExternalTables(String icebergTable, String fields) throws Exception {
        String location = String.format("pxf://%s.%s?PROFILE=iceberg&SERVER=%s", NAMESPACE, icebergTable, PXF_SERVER);
        String writable = "pxf_iceberg_" + icebergTable + "_w";
        String readable = "pxf_iceberg_" + icebergTable + "_r";

        dropIfExists("DROP EXTERNAL TABLE IF EXISTS " + writable);
        dropIfExists("DROP EXTERNAL TABLE IF EXISTS " + readable);
        gpdb.runQuery(String.format("CREATE WRITABLE EXTERNAL TABLE %s (%s) LOCATION ('%s') " +
                "FORMAT 'CUSTOM' (FORMATTER='pxfwritable_export')", writable, fields, location));
        gpdb.runQuery(String.format("CREATE EXTERNAL TABLE %s (%s) LOCATION ('%s') " +
                "FORMAT 'CUSTOM' (FORMATTER='pxfwritable_import')", readable, fields, location));
    }

    /**
     * Runs a DROP ... IF EXISTS statement; runQuery would fail on the "does not exist, skipping" notice.
     */
    private void dropIfExists(String query) throws Exception {
        gpdb.runQuery(query, true, false);
    }

    /**
     * Drops the Iceberg table together with its files, if it exists, and creates it empty.
     */
    private void recreateIcebergTable(String name, String schema, String partitionSpec) throws Exception {
        String tables = "/v1/namespaces/" + NAMESPACE + "/tables";
        rest("DELETE", tables + "/" + name + "?purgeRequested=true", null, 200, 204, 404);

        String body = "{\"name\":\"" + name + "\",\"schema\":" + schema +
                (partitionSpec == null ? "" : ",\"partition-spec\":" + partitionSpec) + "}";
        rest("POST", tables, body, 200);
    }

    private void rest(String method, String path, String body, int... expectedStatuses) throws Exception {
        HttpURLConnection connection = (HttpURLConnection) URI.create(REST_URI + path).toURL().openConnection();
        connection.setConnectTimeout(10000);
        connection.setReadTimeout(60000);
        connection.setRequestMethod(method);
        connection.setRequestProperty("Content-Type", "application/json");
        try {
            if (body != null) {
                connection.setDoOutput(true);
                try (OutputStream output = connection.getOutputStream()) {
                    output.write(body.getBytes(StandardCharsets.UTF_8));
                }
            }
            int responseStatus = connection.getResponseCode();
            for (int status : expectedStatuses) {
                if (responseStatus == status) {
                    return;
                }
            }
            InputStream stream = responseStatus >= 400 ? connection.getErrorStream() : connection.getInputStream();
            ByteArrayOutputStream response = new ByteArrayOutputStream();
            if (stream != null) {
                try (InputStream input = stream) {
                    byte[] buffer = new byte[4096];
                    int count;
                    while ((count = input.read(buffer)) != -1) {
                        response.write(buffer, 0, count);
                    }
                }
            }
            throw new IllegalStateException(String.format("Iceberg REST %s %s returned %d: %s",
                    method, path, responseStatus, response.toString("UTF-8")));
        } finally {
            connection.disconnect();
        }
    }

    private static String field(int id, String name, String type) {
        return String.format("{\"id\":%d,\"name\":\"%s\",\"required\":false,\"type\":\"%s\"}", id, name, type);
    }
}
