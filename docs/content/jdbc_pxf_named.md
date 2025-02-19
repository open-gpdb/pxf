---
title: 'Example: Using a Named Query with PostgreSQL'
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

In this example, you:

- Use the PostgreSQL database `pgtestdb`, user `pxfuser1`, and PXF JDBC connector server configuration `pgsrvcfg` that you created in [Example: Reading From and Writing to a PostgreSQL Database](jdbc_pxf_postgresql.html).
- Create two PostgreSQL tables and insert data into the tables.
- Assign all privileges on the tables to `pxfuser1`.
- Define a named query that performs a complex SQL statement on the two PostgreSQL tables, and add the query to the `pgsrvcfg` JDBC server configuration.
- Create a PXF readable external table definition that matches the query result tuple and also specifies read partitioning options.
- Read the query results, making use of PXF column projection and filter pushdown.

## <a id="nq_create_2pgtbl"></a>Create the PostgreSQL Tables and Assign Permissions

Perform the following procedure to create PostgreSQL tables named `customers` and `orders` in the `public` schema of the database named `pgtestdb`, and grant the user named `pxfuser1` all privileges on these tables:

1. Identify the host name and port of your PostgreSQL server.

2. Connect to the `pgtestdb` PostgreSQL database as the `postgres` user. For example, if your PostgreSQL server is running on the default port on the host named `pserver`:

    ``` shell
    $ psql -U postgres -h pserver -d pgtestdb
    ```

3. Create a table named `customers` and insert some data into this table:

    ``` sql
    CREATE TABLE customers(id int, name text, city text, state text);
    INSERT INTO customers VALUES (111, 'Bill', 'Helena', 'MT');
    INSERT INTO customers VALUES (222, 'Mary', 'Athens', 'OH');
    INSERT INTO customers VALUES (333, 'Tom', 'Denver', 'CO');
    INSERT INTO customers VALUES (444, 'Kate', 'Helena', 'MT');
    INSERT INTO customers VALUES (555, 'Harry', 'Columbus', 'OH');
    INSERT INTO customers VALUES (666, 'Kim', 'Denver', 'CO');
    INSERT INTO customers VALUES (777, 'Erik', 'Missoula', 'MT');
    INSERT INTO customers VALUES (888, 'Laura', 'Athens', 'OH');
    INSERT INTO customers VALUES (999, 'Matt', 'Aurora', 'CO');
    ```

4. Create a table named `orders` and insert some data into this table:

    ``` sql
    CREATE TABLE orders(customer_id int, amount int, month int, year int);
    INSERT INTO orders VALUES (111, 12, 12, 2018);
    INSERT INTO orders VALUES (222, 234, 11, 2018);
    INSERT INTO orders VALUES (333, 34, 7, 2018);
    INSERT INTO orders VALUES (444, 456, 111, 2018);
    INSERT INTO orders VALUES (555, 56, 11, 2018);
    INSERT INTO orders VALUES (666, 678, 12, 2018);
    INSERT INTO orders VALUES (777, 12, 9, 2018);
    INSERT INTO orders VALUES (888, 120, 10, 2018);
    INSERT INTO orders VALUES (999, 120, 11, 2018);
    ```

5. Assign user `pxfuser1` all privileges on tables `customers` and `orders`, and then exit the `psql` subsystem:

    ``` sql
    GRANT ALL ON customers TO pxfuser1;
    GRANT ALL ON orders TO pxfuser1;
    \q
    ```

## <a id="nq_jdbconfig"></a>Configure the Named Query

In this procedure you create a named query text file, add it to the `pgsrvcfg` JDBC server configuration, and synchronize the PXF configuration to the Greenplum Database cluster.

This procedure will typically be performed by the Greenplum Database administrator.

1. Log in to the Greenplum Database coordinator host:

    ``` shell
    $ ssh gpadmin@<coordinator>
    ```

2. Navigate to the JDBC server configuration directory `pgsrvcfg`. For example:

    ``` shell
    gpadmin@coordinator$ cd $PXF_BASE/servers/pgsrvcfg
    ```

3. Open a query text file named `pg_order_report.sql` in a text editor and copy/paste the following query into the file:

    ``` sql
    SELECT c.name, c.city, sum(o.amount) AS total, o.month
      FROM customers c JOIN orders o ON c.id = o.customer_id
      WHERE c.state = 'CO'
    GROUP BY c.name, c.city, o.month
    ```

4. Save the file and exit the editor.

5. Synchronize these changes to the PXF configuration to the Greenplum Database cluster:

    ``` shell
    gpadmin@coordinator$ pxf cluster sync
    ```

## <a id="nq_readjdbc"></a>Read the Query Results

Perform the following procedure on your Greenplum Database cluster to create a PXF external table that references the query file that you created in the previous section, and then reads the query result data:

1. Create the PXF external table specifying the `jdbc` profile. For example:

    ``` sql
    CREATE EXTERNAL TABLE pxf_queryres_frompg(name text, city text, total int, month int)
      LOCATION ('pxf://query:pg_order_report?PROFILE=jdbc&SERVER=pgsrvcfg&PARTITION_BY=month:int&RANGE=1:13&INTERVAL=3')
    FORMAT 'CUSTOM' (FORMATTER='pxfwritable_import');
    ```

    With this partitioning scheme, PXF will issue 4 queries to the remote SQL database, one query per quarter. Each query will return customer names and the total amount of all of their orders in a given month, aggregated per customer, per month, for each month of the target quarter. Greenplum Database will then combine the data into a single result set for you when you query the external table.

2. Display all rows of the query result:

    ``` sql
    SELECT * FROM pxf_queryres_frompg ORDER BY city, total;

     name |  city  | total | month
    ------+--------+-------+-------
     Matt | Aurora |   120 |    11
     Tom  | Denver |    34 |     7
     Kim  | Denver |   678 |    12
    (3 rows)
    ```

3. Use column projection to display the order total per city:

    ``` sql
    SELECT city, sum(total) FROM pxf_queryres_frompg GROUP BY city;

      city  | sum
    --------+-----
     Aurora | 120
     Denver | 712
    (2 rows)
    ```

    When you run this query, PXF requests and retrieves query results for only the `city` and `total` columns, reducing the amount of data sent back to Greenplum Database.

4. Provide additional filters and aggregations to filter the `total` in PostgreSQL:

    ``` sql
    SELECT city, sum(total) FROM pxf_queryres_frompg
                WHERE total > 100
                GROUP BY city;

      city  | sum
    --------+-----
     Denver | 678
     Aurora | 120
    (2 rows)
    ```

    In this example, PXF will add the `WHERE` filter to the subquery. This filter is pushed to and run on the remote database system, reducing the amount of data that PXF sends back to Greenplum Database. The `GROUP BY` aggregation, however, is not pushed to the remote and is performed by Greenplum.

