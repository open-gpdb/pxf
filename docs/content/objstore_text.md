---
title: Reading and Writing Text Data in an Object Store
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

The PXF object store connectors support plain delimited and comma-separated value format text data. This section describes how to use PXF to access text data in an object store, including how to create, query, and insert data into an external table that references files in the object store.

**Note**: Accessing text data from an object store is very similar to accessing text data in HDFS.

## <a id="prereq"></a>Prerequisites

Ensure that you have met the PXF Object Store [Prerequisites](access_objstore.html#objstore_prereq) before you attempt to read data from or write data to an object store.

## <a id="profile_text"></a>Reading Text Data

Use the `<objstore>:text` profile when you read plain text delimited and `<objstore>:csv` when reading .csv data from an object store where each row is a single record.  PXF supports the following `<objstore>` profile prefixes:

| Object Store  | Profile Prefix |
|-------|-------------------------------------|
| Azure Blob Storage   | wasbs |
| Azure Data Lake Storage Gen2    | abfss |
| Google Cloud Storage    | gs |
| MinIO    | s3 |
| S3    | s3 |

The following syntax creates a Greenplum Database readable external table that references a simple text file in an object store: 

``` sql
CREATE EXTERNAL TABLE <table_name> 
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-file>?PROFILE=<objstore>:text|csv&SERVER=<server_name>[&IGNORE_MISSING_PATH=<boolean>][&SKIP_HEADER_COUNT=<numlines>][&<custom-option>=<value>[...]]')
FORMAT '[TEXT|CSV]' (delimiter[=|<space>][E]'<delim_value>');
```

The specific keywords and values used in the Greenplum Database [CREATE EXTERNAL TABLE](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/ref_guide-sql_commands-CREATE_EXTERNAL_TABLE.html) command are described in the table below.

| Keyword  | Value |
|-------|-------------------------------------|
| \<path&#8209;to&#8209;file\>    | The path to the directory or file in the object store. When the `<server_name>` configuration includes a [`pxf.fs.basePath`](cfg_server.html#pxf-fs-basepath) property setting, PXF considers \<path&#8209;to&#8209;file\> to be relative to the base path specified. Otherwise, PXF considers it to be an absolute path. \<path&#8209;to&#8209;file\> must not specify a relative path nor include the dollar sign (`$`) character. |
| PROFILE=\<objstore\>:text<br> PROFILE=\<objstore\>:csv    | The `PROFILE` keyword must identify the specific object store. For example, `s3:text`. |
| SERVER=\<server_name\>    | The named server configuration that PXF uses to access the data. |
| IGNORE_MISSING_PATH=\<boolean\> | Specify the action to take when \<path-to-file\> is missing or invalid. The default value is `false`, PXF returns an error in this situation. When the value is `true`, PXF ignores missing path errors and returns an empty fragment. |
| SKIP_HEADER_COUNT=\<numlines\> | Specify the number of header lines that PXF should skip in the first split of each \<file\> before reading the data. The default value is 0, do not skip any lines. |
| FORMAT | Use `FORMAT` `'TEXT'` when \<path-to-file\> references plain text delimited data.<br> Use `FORMAT` `'CSV'`  when \<path-to-file\> references comma-separated value data.  |
| delimiter    | The delimiter character in the data. For `FORMAT` `'CSV'`, the default \<delim_value\> is a comma (`,`). Preface the \<delim_value\> with an `E` when the value is an escape sequence. Examples: `(delimiter=E'\t')`, `(delimiter ':')`. |

**Note**: PXF does not support the `(HEADER)` formatter option in the `CREATE EXTERNAL TABLE` command. If your text file includes header line(s), use `SKIP_HEADER_COUNT` to specify the number of lines that PXF should skip at the beginning of the first split of each file.

If you are accessing an S3 object store:

- You can provide S3 credentials via custom options in the `CREATE EXTERNAL TABLE` command as described in [Overriding the S3 Server Configuration with DDL](access_s3.html#s3_override).

- If you are reading CSV-format data from S3, you can direct PXF to use the S3 Select Amazon service to retrieve the data. Refer to [Using the Amazon S3 Select Service](access_s3.html#s3_select) for more information about the PXF custom option used for this purpose.

### <a id="profile_text_query"></a>Example: Reading Text Data from S3

Perform the following procedure to create a sample text file, copy the file to S3, and use the `s3:text` and `s3:csv` profiles to create two PXF external tables to query the data.

To run this example, you must:

- Have the AWS CLI tools installed on your system
- Know your AWS access ID and secret key
- Have write permission to an S3 bucket

1. Create a directory in S3 for PXF example data files. For example, if you have write access to an S3 bucket named `BUCKET`:

    ``` shell
    $ aws s3 mb s3://BUCKET/pxf_examples
    ```

2. Locally create a delimited plain text data file named `pxf_s3_simple.txt`:

    ``` shell
    $ echo 'Prague,Jan,101,4875.33
    Rome,Mar,87,1557.39
    Bangalore,May,317,8936.99
    Beijing,Jul,411,11600.67' > /tmp/pxf_s3_simple.txt
    ```

    Note the use of the comma (`,`) to separate the four data fields.

4. Copy the data file to the S3 directory you created in Step 1:

    ``` shell
    $ aws s3 cp /tmp/pxf_s3_simple.txt s3://BUCKET/pxf_examples/
    ```

5. Verify that the file now resides in S3:

    ``` shell
    $ aws s3 ls s3://BUCKET/pxf_examples/pxf_s3_simple.txt
    ```

4. Start the `psql` subsystem:

    ``` shell
    $ psql -d postgres
    ```

1. Use the PXF `s3:text` profile to create a Greenplum Database external table that references the `pxf_s3_simple.txt` file that you just created and added to S3. For example, if your server name is `s3srvcfg`:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_textsimple(location text, month text, num_orders int, total_sales float8)
                LOCATION ('pxf://BUCKET/pxf_examples/pxf_s3_simple.txt?PROFILE=s3:text&SERVER=s3srvcfg')
              FORMAT 'TEXT' (delimiter=E',');
    ```
              
2. Query the external table:

    ``` sql
    postgres=# SELECT * FROM pxf_s3_textsimple;          
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

2. Create a second external table that references `pxf_s3_simple.txt`, this time specifying the `s3:csv` `PROFILE` and the `CSV` `FORMAT`:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_textsimple_csv(location text, month text, num_orders int, total_sales float8)
                LOCATION ('pxf://BUCKET/pxf_examples/pxf_s3_simple.txt?PROFILE=s3:csv&SERVER=s3srvcfg')
              FORMAT 'CSV';
    postgres=# SELECT * FROM pxf_s3_textsimple_csv;          
    ```

    When you specify `FORMAT 'CSV'` for comma-separated value data, no `delimiter` formatter option is required because comma is the default delimiter value.

## <a id="profile_textmulti"></a>Reading Text Data with Quoted Linefeeds

Use the `<objstore>:text:multi` profile to read plain text data with delimited single- or multi- line records that include embedded (quoted) linefeed characters. The following syntax creates a Greenplum Database readable external table that references such a text file in an object store:

``` sql
CREATE EXTERNAL TABLE <table_name>
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-file>?PROFILE=<objstore>:text:multi&SERVER=<server_name>[&IGNORE_MISSING_PATH=<boolean>][&SKIP_HEADER_COUNT=<numlines>][&<custom-option>=<value>[...]]')
FORMAT '[TEXT|CSV]' (delimiter[=|<space>][E]'<delim_value>');
```

The specific keywords and values used in the [CREATE EXTERNAL TABLE](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/ref_guide-sql_commands-CREATE_EXTERNAL_TABLE.html) command are described in the table below.

| Keyword  | Value |
|-------|-------------------------------------|
| \<path&#8209;to&#8209;file\>    | The path to the directory or file in the data store. When the `<server_name>` configuration includes a [`pxf.fs.basePath`](cfg_server.html#pxf-fs-basepath) property setting, PXF considers \<path&#8209;to&#8209;file\> to be relative to the base path specified. Otherwise, PXF considers it to be an absolute path. \<path&#8209;to&#8209;file\> must not specify a relative path nor include the dollar sign (`$`) character. |
| PROFILE=\<objstore\>:text:multi    | The `PROFILE` keyword must identify the specific object store. For example, `s3:text:multi`. |
| SERVER=\<server_name\>    | The named server configuration that PXF uses to access the data. |
| IGNORE_MISSING_PATH=\<boolean\> | Specify the action to take when \<path-to-file\> is missing or invalid. The default value is `false`, PXF returns an error in this situation. When the value is `true`, PXF ignores missing path errors and returns an empty fragment. |
| SKIP_HEADER_COUNT=\<numlines\> | Specify the number of header lines that PXF should skip in the first split of each \<file\> before reading the data. The default value is 0, do not skip any lines. |
| FORMAT | Use `FORMAT` `'TEXT'` when \<path-to-file\> references plain text delimited data.<br> Use `FORMAT` `'CSV'` when \<path-to-file\> references comma-separated value data.  |
| delimiter    | The delimiter character in the data. For `FORMAT` `'CSV'`, the default \<delim_value\> is a comma (`,`). Preface the \<delim_value\> with an `E` when the value is an escape sequence. Examples: `(delimiter=E'\t')`, `(delimiter ':')`. |

**Note**: PXF does not support the `(HEADER)` formatter option in the `CREATE EXTERNAL TABLE` command. If your text file includes header line(s), use `SKIP_HEADER_COUNT` to specify the number of lines that PXF should skip at the beginning of the first split of each file.

If you are accessing an S3 object store, you can provide S3 credentials via custom options in the `CREATE EXTERNAL TABLE` command as described in [Overriding the S3 Server Configuration with DDL](access_s3.html#s3_override).

### <a id="profile_textmulti_query"></a>Example: Reading Multi-Line Text Data from S3

Perform the following steps to create a sample text file, copy the file to S3, and use the PXF `s3:text:multi` profile to create a Greenplum Database readable external table to query the data.

To run this example, you must:

- Have the AWS CLI tools installed on your system
- Know your AWS access ID and secret key
- Have write permission to an S3 bucket

1. Create a second delimited plain text file:

    ``` shell
    $ vi /tmp/pxf_s3_multi.txt
    ```

2. Copy/paste the following data into `pxf_s3_multi.txt`:

    ``` pre
    "4627 Star Rd.
    San Francisco, CA  94107":Sept:2017
    "113 Moon St.
    San Diego, CA  92093":Jan:2018
    "51 Belt Ct.
    Denver, CO  90123":Dec:2016
    "93114 Radial Rd.
    Chicago, IL  60605":Jul:2017
    "7301 Brookview Ave.
    Columbus, OH  43213":Dec:2018
    ```

    Notice the use of the colon `:` to separate the three fields. Also notice the quotes around the first (address) field. This field includes an embedded line feed separating the street address from the city and state.

3. Copy the text file to S3:

    ``` shell
    $ aws s3 cp /tmp/pxf_s3_multi.txt s3://BUCKET/pxf_examples/
    ```

4. Use the `s3:text:multi` profile to create an external table that references the `pxf_s3_multi.txt` S3 file, making sure to identify the `:` (colon) as the field separator. For example, if your server name is `s3srvcfg`:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_textmulti(address text, month text, year int)
                LOCATION ('pxf://BUCKET/pxf_examples/pxf_s3_multi.txt?PROFILE=s3:text:multi&SERVER=s3srvcfg')
              FORMAT 'CSV' (delimiter ':');
    ```
    
    Notice the alternate syntax for specifying the `delimiter`.
    
2. Query the `pxf_s3_textmulti` table:

    ``` sql
    postgres=# SELECT * FROM pxf_s3_textmulti;
    ```

    ``` pre
             address          | month | year 
    --------------------------+-------+------
     4627 Star Rd.            | Sept  | 2017
     San Francisco, CA  94107           
     113 Moon St.             | Jan   | 2018
     San Diego, CA  92093               
     51 Belt Ct.              | Dec   | 2016
     Denver, CO  90123                  
     93114 Radial Rd.         | Jul   | 2017
     Chicago, IL  60605                 
     7301 Brookview Ave.      | Dec   | 2018
     Columbus, OH  43213                
    (5 rows)
    ```

## <a id="write_text"></a>Writing Text Data

The `<objstore>:text|csv` profiles support writing single line plain text data to an object store. When you create a writable external table with PXF, you specify the name of a directory. When you insert records into a writable external table, the block(s) of data that you insert are written to one or more files in the directory that you specified.

**Note**: External tables that you create with a writable profile can only be used for `INSERT` operations. If you want to query the data that you inserted, you must create a separate readable external table that references the directory.

Use the following syntax to create a Greenplum Database writable external table that references an object store directory: 

``` sql
CREATE WRITABLE EXTERNAL TABLE <table_name> 
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-dir>
    ?PROFILE=<objstore>:text|csv&SERVER=<server_name>[&<custom-option>=<value>[...]]')
FORMAT '[TEXT|CSV]' (delimiter[=|<space>][E]'<delim_value>');
[DISTRIBUTED BY (<column_name> [, ... ] ) | DISTRIBUTED RANDOMLY];
```

The specific keywords and values used in the [CREATE EXTERNAL TABLE](https://docs.vmware.com/en/VMware-Greenplum/6/greenplum-database/ref_guide-sql_commands-CREATE_EXTERNAL_TABLE.html) command are described in the table below.

| Keyword  | Value |
|-------|-------------------------------------|
| \<path&#8209;to&#8209;dir\>    | The path to the directory in the data store. When the `<server_name>` configuration includes a [`pxf.fs.basePath`](cfg_server.html#pxf-fs-basepath) property setting, PXF considers \<path&#8209;to&#8209;dir\> to be relative to the base path specified. Otherwise, PXF considers it to be an absolute path. \<path&#8209;to&#8209;dir\> must not specify a relative path nor include the dollar sign (`$`) character. |
| PROFILE=\<objstore\>:text<br> PROFILE=\<objstore\>:csv    | The `PROFILE` keyword must identify the specific object store. For example, `s3:text`. |
| SERVER=\<server_name\>    | The named server configuration that PXF uses to access the data. |
| \<custom&#8209;option\>=\<value\>  | \<custom-option\>s are described below.|
| FORMAT | Use `FORMAT` `'TEXT'` to write plain, delimited text to \<path-to-dir\>.<br> Use `FORMAT` `'CSV'` to write comma-separated value text to \<path-to-dir\>. |
| delimiter    | The delimiter character in the data. For `FORMAT` `'CSV'`, the default \<delim_value\> is a comma (`,`). Preface the \<delim_value\> with an `E` when the value is an escape sequence. Examples: `(delimiter=E'\t')`, `(delimiter ':')`. |
| DISTRIBUTED BY    | If you want to load data from an existing Greenplum Database table into the writable external table, consider specifying the same distribution policy or `<column_name>` on both tables. Doing so will avoid extra motion of data between segments on the load operation. |

Writable external tables that you create using an `<objstore>:text|csv` profile can optionally use record or block compression. You specify the compression codec via a custom option in the `CREATE EXTERNAL TABLE` `LOCATION` clause. The `<objstore>:text|csv` profiles support the following custom write options:

| Option  | Value Description |
|-------|-------------------------------------|
| COMPRESSION_CODEC    | The compression codec alias. Supported compression codecs for writing text data include: `default`, `bzip2`, `gzip`, and `uncompressed`. If this option is not provided, Greenplum Database performs no data compression. |

If you are accessing an S3 object store, you can provide S3 credentials via custom options in the `CREATE EXTERNAL TABLE` command as described in [Overriding the S3 Server Configuration with DDL](access_s3.html#s3_override).

### <a id="write_s3textsimple_example"></a>Example: Writing Text Data to S3

This example utilizes the data schema introduced in [Example: Reading Text Data from S3](#profile_text_query). 

| Column Name  | Data Type |
|-------|-------------------------------------|
| location | text |
| month | text |
| number\_of\_orders | int |
| total\_sales | float8 |

This example also optionally uses the Greenplum Database external table named `pxf_s3_textsimple` that you created in that exercise.

#### <a id="write_s3textsimple_proc" class="no-quick-link"></a>Procedure

Perform the following procedure to create Greenplum Database writable external tables utilizing the same data schema as described above, one of which will employ compression. You will use the PXF `s3:text` profile to write data to S3. You will also create a separate, readable external table to read the data that you wrote to S3.

1. Create a Greenplum Database writable external table utilizing the data schema described above. Write to the S3 directory `BUCKET/pxf_examples/pxfwrite_s3_textsimple1`. Create the table specifying a comma (`,`) as the delimiter. For example, if your server name is `s3srvcfg`:

    ``` sql
    postgres=# CREATE WRITABLE EXTERNAL TABLE pxf_s3_writetbl_1(location text, month text, num_orders int, total_sales float8)
                LOCATION ('pxf://BUCKET/pxf_examples/pxfwrite_s3_textsimple1?PROFILE=s3:text|csv&SERVER=s3srvcfg')
              FORMAT 'TEXT' (delimiter=',');
    ```
    
    You specify the `FORMAT` subclause `delimiter` value as the single ascii comma character `,`.

2. Write a few individual records to the `pxfwrite_s3_textsimple1` S3 directory by invoking the SQL `INSERT` command on `pxf_s3_writetbl_1`:

    ``` sql
    postgres=# INSERT INTO pxf_s3_writetbl_1 VALUES ( 'Frankfurt', 'Mar', 777, 3956.98 );
    postgres=# INSERT INTO pxf_s3_writetbl_1 VALUES ( 'Cleveland', 'Oct', 3812, 96645.37 );
    ```

3. (Optional) Insert the data from the `pxf_s3_textsimple` table that you created in [Example: Reading Text Data from S3] (#profile_text_query) into `pxf_s3_writetbl_1`:

    ``` sql
    postgres=# INSERT INTO pxf_s3_writetbl_1 SELECT * FROM pxf_s3_textsimple;
    ```

5. Greenplum Database does not support directly querying a writable external table. To query the data that you just added to S3, you must create a readable external Greenplum Database table that references the S3 directory:

    ``` sql
    postgres=# CREATE EXTERNAL TABLE pxf_s3_textsimple_r1(location text, month text, num_orders int, total_sales float8)
                LOCATION ('pxf://BUCKET/pxf_examples/pxfwrite_s3_textsimple1?PROFILE=s3:text&SERVER=s3srvcfg')
			    FORMAT 'CSV';
    ```

    You specify the `'CSV'` `FORMAT` when you create the readable external table because you created the writable table with a comma (`,`) as the delimiter character, the default delimiter for `'CSV'` `FORMAT`.

6. Query the readable external table:

    ``` sql
    postgres=# SELECT * FROM pxf_s3_textsimple_r1 ORDER BY total_sales;
    ```

    ``` pre
     location  | month | num_orders | total_sales 
    -----------+-------+------------+-------------
     Rome      | Mar   |         87 |     1557.39
     Frankfurt | Mar   |        777 |     3956.98
     Prague    | Jan   |        101 |     4875.33
     Bangalore | May   |        317 |     8936.99
     Beijing   | Jul   |        411 |    11600.67
     Cleveland | Oct   |       3812 |    96645.37
    (6 rows)
    ```

    The `pxf_s3_textsimple_r1` table includes the records you individually inserted, as well as the full contents of the `pxf_s3_textsimple` table if you performed the optional step.

7. Create a second Greenplum Database writable external table, this time using Gzip compression and employing a colon `:` as the delimiter:

    ``` sql
    postgres=# CREATE WRITABLE EXTERNAL TABLE pxf_s3_writetbl_2 (location text, month text, num_orders int, total_sales float8)
                LOCATION ('pxf://BUCKET/pxf_examples/pxfwrite_s3_textsimple2?PROFILE=s3:text&SERVER=s3srvcfg&COMPRESSION_CODEC=gzip')
              FORMAT 'TEXT' (delimiter=':');
    ```

8. Write a few records to the `pxfwrite_s3_textsimple2` S3 directory by inserting directly into the `pxf_s3_writetbl_2` table:

    ``` sql
    gpadmin=# INSERT INTO pxf_s3_writetbl_2 VALUES ( 'Frankfurt', 'Mar', 777, 3956.98 );
    gpadmin=# INSERT INTO pxf_s3_writetbl_2 VALUES ( 'Cleveland', 'Oct', 3812, 96645.37 );
    ```

9. To query data from the newly-created S3 directory named `pxfwrite_s3_textsimple2`, you can create a readable external Greenplum Database table as described above that references this S3 directory and specifies `FORMAT 'CSV' (delimiter=':')`.


## <a id="multibyte_delim"></a>About Reading Data Containing Multi-Byte or Multi-Character Delimiters

You can use only a `*:csv` PXF profile to read data from an object store that contains a multi-byte delimiter or a delimiter with multiple characters. The syntax for creating a readable external table for such data follows:

``` sql
CREATE EXTERNAL TABLE <table_name>
    ( <column_name> <data_type> [, ...] | LIKE <other_table> )
LOCATION ('pxf://<path-to-file>?PROFILE=<objstore>:csv[&SERVER=<server_name>][&IGNORE_MISSING_PATH=<boolean>][&SKIP_HEADER_COUNT=<numlines>][&NEWLINE=<bytecode>]')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import' <option>[=|<space>][E]'<value>');
```

Note the `FORMAT` line in the syntax block. While the syntax is similar to that of reading CSV, PXF requires a custom formatter to read data containing a multi-byte or multi-character delimiter. You must specify the `'CUSTOM'` format and the `pxfdelimited_import` formatter. You must also specify a delimiter in the formatter options.

PXF recognizes the following formatter options when reading data from an object store that contains a multi-byte or multi-character delimiter:

| Option Name  | Value Description | Default Value |
|-------|---------------|---------------|
| DELIMITER=\<delim_string\> | The single-byte or multi-byte delimiter string that separates columns. The string may be up to 32 bytes in length, and may not contain quote or escape characters. **Required** | None |
| QUOTE=\<char\> | The single one-byte ASCII quotation character for all columns. | None |
| ESCAPE=\<char\> | The single one-byte ASCII character used to escape special characters (for example, the `DELIM`, `QUOTE`,  or `NEWLINE` value, or the `ESCAPE` value itself). | None, or the `QUOTE` value if that is set |
| NEWLINE=\<bytecode\> | The end-of-line indicator that designates the end of a row. Valid values are `LF` (line feed), `CR` (carriage return), or `CRLF` (carriage return plus line feed. | `LF` |

The following sections provide further information about, and examples for, specifying the delimiter, quote, escape, and new line options.

### <a id="about_delim"></a>Specifying the Delimiter

You must directly specify the delimiter or provide its byte representation. For example, given the following sample data that uses a `¤` currency symbol delimiter:

```
133¤Austin¤USA
321¤Boston¤USA
987¤Paris¤France
```

To read this data from S3 using a PXF server configuration named `s3srvcfg`, create the external table as follows:

```
CREATE READABLE EXTERNAL TABLE s3_mbyte_delim (id int, city text, country text)
  LOCATION ('pxf://multibyte_currency?PROFILE=s3:csv&SERVER=s3srvcfg')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import', DELIMITER='¤'); 
```

#### <a id="delim_byterep"></a>About Specifying the Byte Representation of the Delimiter

You can directly specify the delimiter or provide its byte representation. If you choose to specify the byte representation of the delimiter:

- You must specify the byte representation of the delimiter in `E'<value>'` format.
- Because some characters have different byte representations in different encodings, you must specify the byte representation of the delimiter in the *database encoding*.

For example, if the database encoding is `UTF8`, the file encoding is `LATIN1`, and the delimiter is the `¤` currency symbol, you must specify the `UTF8` byte representation for `¤`, which is `\xC2\xA4`:

```
CREATE READABLE EXTERNAL TABLE s3_byterep_delim (id int, city text, country text)
  LOCATION ('pxf://multibyte_example?PROFILE=s3:csv&SERVER=s3srvcfg')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import', DELIMITER=E'\xC2\xA4') ENCODING 'LATIN1';
```

### <a id="about_qe"></a>About Specifying Quote and Escape Characters

When PXF reads data that contains a multi-byte or multi-character delimiter, its behavior depends on the quote and escape character settings:

| QUOTE Set? | ESCAPE Set? | PXF Behaviour |
|-------------|--------------|----------|
| No<sup>1</sup> | No | PXF reads the data as-is. |
| Yes<sup>2</sup> | Yes | PXF reads the data between quote characters as-is and un-escapes only the quote and escape characters. |
| Yes<sup>2</sup> | No (`ESCAPE 'OFF'`) | PXF reads the data between quote characters as-is. |
| No<sup>1</sup> | Yes | PXF reads the data as-is and un-escapes only the delimiter, newline, and escape itself. |

<sup>1</sup> All data columns must be un-quoted when you do not specify a quote character.

<sup>2</sup> All data columns must quoted when you specify a quote character.

> **Note** PXF expects that there are no extraneous characters between the quote value and the delimiter value, nor between the quote value and the end-of-line value. Additionally, there must be no white space between delimiters and quotes.

### <a id="about_newline"></a>About the NEWLINE Options

PXF requires that every line in the file be terminated with the same new line value.

By default, PXF uses the line feed character (`LF`) for the new line delimiter. When the new line delimiter for the external file is also a line feed, you need not specify the `NEWLINE` formatter option.

If the `NEWLINE` formatter option is provided and contains `CR` or `CRLF`, you must also specify the same `NEWLINE` option in the external table `LOCATION` URI. For example, if the new line delimiter is `CRLF`, create the external table as follows:

```
CREATE READABLE EXTERNAL TABLE s3_mbyte_newline_crlf (id int, city text, country text)
  LOCATION ('pxf://multibyte_example_crlf?PROFILE=s3:csv&SERVER=s3srvcfg&NEWLINE=CRLF')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import', DELIMITER='¤', NEWLINE='CRLF');
```

### <a id="mbd_examples"></a>Examples

#### <a id="qe_example"></a>Delimiter with Quoted Data

Given the following sample data that uses the double-quote (`"`) quote character and the delimiter `¤`:

```
"133"¤"Austin"¤"USA"
"321"¤"Boston"¤"USA"
"987"¤"Paris"¤"France"
```

Create the external table as follows:

```
CREATE READABLE EXTERNAL TABLE s3_mbyte_delim_quoted (id int, city text, country text)
  LOCATION ('pxf://multibyte_q?PROFILE=s3:csv&SERVER=s3srvcfg')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import', DELIMITER='¤', QUOTE '"'); 
```

#### <a id="qe_example"></a>Delimiter with Quoted and Escaped Data

Given the following sample data that uses the quote character `"`, the escape character `\`, and the delimiter `¤`:

```
"\"hello, my name is jane\" she said. let's escape something \\"¤"123"
```

Create the external table as follows:

```
CREATE READABLE EXTERNAL TABLE s3_mybte_delim_quoted_escaped (sentence text, num int)
  LOCATION ('pxf://multibyte_qe?PROFILE=s3:csv&SERVER=s3srvcfg')
FORMAT 'CUSTOM' (FORMATTER='pxfdelimited_import', DELIMITER='¤', QUOTE '"', ESCAPE '\');
```

With this external table definition, PXF reads the `sentence` text field as:

```
SELECT sentence FROM s3_mbyte_delim_quoted_escaped;

                          sentence 
-------------------------------------------------------------
 "hello, my name is jane" she said. let's escape something \
(1 row)
```

