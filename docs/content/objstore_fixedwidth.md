---
title: Reading and Writing Fixed-Width Text Data in an Object Store
---

The PXF object store connectors support reading and writing fixed-width text using the Greenplum Database [fixed width custom formatter](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/admin_guide-load-topics-g-importing-and-exporting-fixed-width-data.html). This section describes how to use PXF to access fixed-width text, including how to create, query, and insert data into an external table that references files in the object store.

**Note**: Accessing fixed-width text data from an object store is very similar to accessing such data in HDFS.

## <a id="prereq"></a>Prerequisites

Ensure that you have met the PXF Object Store [Prerequisites](access_objstore.html#objstore_prereq) before you attempt to read data from or write data to an object store.

## <a id="profile_fixedwidth"></a>Reading Text Data with Fixed Widths

Use the `<objstore>:fixedwidth` profile when you read fixed-width text from an object store where each line is a single record.  PXF supports the following `<objstore>` profile prefixes:

| Object Store  | Profile Prefix |
|-------|-------------------------------------|
| Azure Blob Storage   | wasbs |
| Azure Data Lake Storage Gen2    | abfss |
| Google Cloud Storage    | gs |
| MinIO    | s3 |
| AWS S3    | s3 |

The following syntax creates a Greenplum Database readable external table that references such a text file in an object store: 

``` sql
CREATE EXTERNAL TABLE <table_name> 
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-file>?PROFILE=<objstore>:fixedwidth[&SERVER=<server_name>][&NEWLINE=<bytecode>][&IGNORE_MISSING_PATH=<boolean>]')
FORMAT 'CUSTOM' (FORMATTER='fixedwidth_in', <field_name>='<width>' [, ...] [, line_delim[=|<space>][E]'<delim_value>']);
```

The specific keywords and values used in the Greenplum Database [CREATE EXTERNAL TABLE](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/ref_guide-sql_commands-CREATE_EXTERNAL_TABLE.html) command are described in the table below.

| Keyword  | Value |
|-------|-------------------------------------|
| \<path&#8209;to&#8209;file\>    | The path to the directory or file in the object store. When the `<server_name>` configuration includes a [`pxf.fs.basePath`](cfg_server.html#pxf-fs-basepath) property setting, PXF considers \<path&#8209;to&#8209;file\> to be relative to the base path specified. Otherwise, PXF considers it to be an absolute path. \<path&#8209;to&#8209;file\> must not specify a relative path nor include the dollar sign (`$`) character. |
| PROFILE=\<objstore\>:fixedwidth | The `PROFILE` must identify the specific object store. For example, `s3:fixedwidth`. |
| SERVER=\<server_name\>    | The named server configuration that PXF uses to access the data. PXF uses the `default` server if not specified. |
| NEWLINE=\<bytecode\>    | When the `line_delim` formatter option contains `\r`, `\r\n`, or a set of custom escape characters, you must set `<bytecode>` to `CR`, `CRLF`, or the set of bytecode characters, respectively. |
| IGNORE_MISSING_PATH=\<boolean\> | Specify the action to take when \<path-to-file\> is missing or invalid. The default value is `false`, PXF returns an error in this situation. When the value is `true`, PXF ignores missing path errors and returns an empty fragment. |
| FORMAT 'CUSTOM' | Use `FORMAT` '`CUSTOM`' with `FORMATTER='fixedwidth_in'` (read). |
| \<field_name>='\<width>'    | The name and the width of the field. For example: `first_name='15'` specifies that the `first_name` field is `15` characters long. By default, when the field value is less than `<width>` size, Greenplum Database expects the field to be right-padded with spaces to that size. |
| line_delim    | The line delimiter character in the data. Preface the \<delim_value\> with an `E` when the value is an escape sequence. Examples: `line_delim=E'\n'`, `line_delim 'aaa'`. The default value is `'\n'`.|

**Note**: PXF does not support the `(HEADER)` formatter option in the `CREATE EXTERNAL TABLE` command.

If you are accessing an S3 object store, you can provide S3 credentials via custom options in the `CREATE EXTERNAL TABLE` command as described in [Overriding the S3 Server Configuration with DDL](access_s3.html#s3_override).

## <a id="about_fields"></a>About Specifying field_name and width

Greenplum Database loads all fields in a line of fixed-width data in their physical order. The `<field_name>`s that you specify in the `FORMAT` options must match the order that you define the columns in the `CREATE [WRITABLE] EXTERNAL TABLE` command. You specify the size of each field in the `<width>` value.

Refer to the Greenplum Database [fixed width custom formatter documentation](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/admin_guide-load-topics-g-importing-and-exporting-fixed-width-data.html) for more information about the formatter options.


## <a id="about_lineend"></a>About the line_delim and NEWLINE Formatter Options

By default, Greenplum Database uses the `\n` (LF) character for the new line delimiter. When the line delimiter for the external file is also `\n`, you need not specify the `line_delim` option. If the `line_delim` formatter option is provided and contains `\r` (CR), `\r\n` (CRLF), or a set of custom escape characters, you must specify the `NEWLINE` option in the external table `LOCATION` clause, and set the value to `CR`, `CRLF`, or the set of bytecode characters, respectively.

Refer to the Greenplum Database [fixed width custom formatter documentation](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/admin_guide-load-topics-g-importing-and-exporting-fixed-width-data.html) for more information about the formatter options.

## <a id="fixedwidth_read_example"></a>Example: Reading Fixed-Width Text Data on S3

Perform the following procedure to create a sample text file, copy the file to S3, and use the `s3:fixedwidth` profile to create a PXF external table to query the data.

To run this example, you must:

- Have the AWS CLI tools installed on your system
- Know your AWS access ID and secret key
- Have write permission to an S3 bucket

Procedure:

1. Create a directory in S3 for PXF example data files. For example, if you have write access to an S3 bucket named `BUCKET`:

    ``` shell
    $ aws s3 mb s3://BUCKET/pxf_examples
    ```
1. Locally create a plain text data file named `pxf_s3_fixedwidth.txt`:

    ``` shell
    $ echo 'Prague         Jan 101   4875.33   
    Rome           Mar 87    1557.39   
    Bangalore      May 317   8936.99   
    Beijing        Jul 411   11600.67  ' > /tmp/pxf_s3_fixedwidth.txt
    ```

    In this sample file, the first field is 15 characters long, the second is 4 characters, the third is 6 characters, and the last field is 10 characters long.

    > **Note** Open the `/tmp/pxf_s3_fixedwidth.txt` file in the editor of your choice, and ensure that the last field is right-padded with spaces to 10 characters in size.

1. Copy the data file to the S3 directory that you created in Step 1:

    ``` shell
    $ aws s3 cp /tmp/pxf_s3_fixedwidth.txt s3://BUCKET/pxf_examples/
    ```

1. Verify that the file now resides in S3:

    ``` shell
    $ aws s3 ls s3://BUCKET/pxf_examples/pxf_s3s_fixedwidth.txt
    ```

1. Start the `psql` subsystem:

    ``` shell
    $ psql -d postgres
    ```

1. Use the PXF `s3:fixedwidth` profile to create a Greenplum Database external table that references the `pxf_s3_fixedwidth.txt` file that you just created and added to S3. For example, if your server name is `s3srvcfg`:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_fixedwidth_r(location text, month text, num_orders int, total_sales float8)
                 LOCATION ('pxf://data/pxf_examples/pxf_s3_fixedwidth.txt?PROFILE=s3:fixedwidth&SERVER=s3srvcfg&NEWLINE=CRLF')
               FORMAT 'CUSTOM' (formatter='fixedwidth_in', location='15', month='4', num_orders='6', total_sales='10', line_delim=E'\r\n');
    ```
              
2. Query the external table:

    ``` sql
    postgres=# SELECT * FROM pxf_s3_fixedwidth_r;
    ```

    ``` pre
       location    | month | num_orders | total_sales 
    ---------------+-------+------------+-------------
     Prague        | Jan   |        101 |     4875.33
     Rome          | Mar   |         87 |     1557.39
     Bangalore     | May   |        317 |     8936.99
     Beijing       | Jul   |        411 |    11600.67
    (4 rows)
    ```

## <a id="s3write_text"></a>Writing Fixed-Width Text Data

The `<objstore>:fixedwidth` profiles support writing fixed-width text to an object store. When you create a writable external table with PXF, you specify the name of a directory. When you insert records into a writable external table, the block(s) of data that you insert are written to one or more files in the directory that you specified.

**Note**: External tables that you create with a writable profile can only be used for `INSERT` operations. If you want to query the data that you inserted, you must create a separate readable external table that references the directory.

Use the following syntax to create a Greenplum Database writable external table that references an object store directory: 

``` sql
CREATE WRITABLE EXTERNAL TABLE <table_name> 
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-dir>
    ?PROFILE=<objstore>:fixedwidth[&SERVER=<server_name>][&NEWLINE=<bytecode>][&<write-option>=<value>[...]]')
FORMAT 'CUSTOM' (FORMATTER='fixedwidth_out' [, <field_name>='<width>'] [, ...] [, line_delim[=|<space>][E]'<delim_value>']);
[DISTRIBUTED BY (<column_name> [, ... ] ) | DISTRIBUTED RANDOMLY];
```

The specific keywords and values used in the [CREATE EXTERNAL TABLE](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/ref_guide-sql_commands-CREATE_EXTERNAL_TABLE.html) command are described in the table below.

| Keyword  | Value |
|-------|-------------------------------------|
| \<path&#8209;to&#8209;dir\>    | The path to the directory in the data store. When the `<server_name>` configuration includes a [`pxf.fs.basePath`](cfg_server.html#pxf-fs-basepath) property setting, PXF considers \<path&#8209;to&#8209;dir\> to be relative to the base path specified. Otherwise, PXF considers it to be an absolute path. \<path&#8209;to&#8209;dir\> must not specify a relative path nor include the dollar sign (`$`) character. |
| PROFILE=\<objstore\>:fixedwidth | The `PROFILE` must identify the specific object store. For example, `s3:fixedwidth`. |
| SERVER=\<server_name\>    | The named server configuration that PXF uses to access the data. PXF uses the `default` server if not specified. |
| NEWLINE=\<bytecode\>    | When the `line_delim` formatter option contains `\r`, `\r\n`, or a set of custom escape characters, you must set `<bytecode>` to `CR`, `CRLF`, or the set of bytecode characters, respectively. |
| \<write&#8209;option\>=\<value\>  | \<write-option\>s are described below.|
| FORMAT 'CUSTOM' | Use `FORMAT` '`CUSTOM`' with `FORMATTER='fixedwidth_out'` (write). |
| \<field_name>='\<width>'    | The name and the width of the field. For example: `first_name='15'` specifies that the `first_name` field is `15` characters long. By default, when writing to the external file and the field value is less than `<width>` size, Greenplum Database right-pads the field with spaces to `<width>` size. |
| line_delim    | The line delimiter character in the data. Preface the \<delim_value\> with an `E` when the value is an escape sequence. Examples: `line_delim=E'\n'`, `line_delim 'aaa'`. The default value is `'\n'`. |
| DISTRIBUTED BY    | If you want to load data from an existing Greenplum Database table into the writable external table, consider specifying the same distribution policy or `<column_name>` on both tables. Doing so will avoid extra motion of data between segments on the load operation. |

Writable external tables that you create using the `<objstore>:fixedwidth` profile can optionally use record or block compression. You specify the compression codec via an option in the `CREATE WRITABLE EXTERNAL TABLE` `LOCATION` clause:

| Write Option  | Value Description |
|-------|-------------------------------------|
| COMPRESSION_CODEC    | The compression codec alias. Supported compression codecs for writing fixed-width text data include: `default`, `bzip2`, `gzip`, and `uncompressed`. If this option is not provided, Greenplum Database performs no data compression. |

## <a id="fixedwidth_write_example"></a>Example: Writing Fixed-Width Text Data to S3

This example utilizes the data schema introduced in [Example: Reading Fixed-Width Text Data on S3](#fixedwidth_read_example). 

| Column Name  | Width | Data Type |
|---|---|---|
| location | 15 | text |
| month | 4 | text |
| number_of_orders | 6 | int |
| total_sales | 10 | float8 |

#### <a id="fixedwidth_write_proc" class="no-quick-link"></a>Procedure

Perform the following procedure to create a Greenplum Database writable external table utilizing the same data schema as described above. You will use the PXF `s3:fixedwidth` profile to write data to S3. You will also create a separate, readable external table to read the data that you wrote to #3.

1. Create a Greenplum Database writable external table utilizing the data schema described above. Write to the S3 directory `BUCKET/pxf_examples/fixedwidth_write`. Create the table specifying `\n` as the line delimiter. For example, if your server name is `s3srvcfg`:

    ``` sql
    postgres=# CREATE WRITABLE EXTERNAL TABLE pxf_s3_fixedwidth_w(location text, month text, num_orders int, total_sales float8)
                 LOCATION ('pxf://BUCKET/pxf_examples/fixedwidth_write?PROFILE=s3:fixedwidth&SERVER=s3srvcfg')
               FORMAT 'CUSTOM' (formatter='fixedwidth_out', location='15', month='4', num_orders='6', total_sales='10');
    ```
    
2. Write a few individual records to the `fixedwidth_write` S3 directory by using the `INSERT` command on the `pxf_s3_fixedwidth_w` table:

    ``` sql
    postgres=# INSERT INTO pxf_s3_fixedwidth_w VALUES ( 'Frankfurt', 'Mar', 777, 3956.98 );
    postgres=# INSERT INTO pxf_s3_fixedwidth_w VALUES ( 'Cleveland', 'Oct', 3812, 96645.37 );
    ```

5. Greenplum Database does not support directly querying a writable external table. To query the data that you just added to S3, you must create a readable external Greenplum Database table that references the S3 directory:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_fixedwidth_r2(location text, month text, num_orders int, total_sales float8)
                 LOCATION ('pxf://BUCKET/pxf_examples/fixedwidth_write?PROFILE=s3:fixedwidth&SERVER=s3srvcfg')
               FORMAT 'CUSTOM' (formatter='fixedwidth_in', location='15', month='4', num_orders='6', total_sales='10');
    ```

6. Query the readable external table:

    ``` sql
    postgres=# SELECT * FROM pxf_s3_fixedwidth_r2 ORDER BY total_sales;
    ```

    ``` pre
     location  | month | num_orders | total_sales 
    -----------+-------+------------+-------------
     Frankfurt | Mar   |        777 |     3956.98
     Cleveland | Oct   |       3812 |    96645.37
    (2 rows)
    ```

