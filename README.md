# pgexodus a PostgreSQL Extension

TODO: Reimplement extract and custom data types separately.

Provide conversions from text instead of performing extract and conversion in one step.

## Reference

https://www.postgresql.org/docs/current/extend-extensions.html

## Provides

Dependencies of the EXODUS database select statement which depends on the following functions:

* exodus.extract_text
* exodus.extract_number
* exodus.extract_date
* exodus.extract_time
* exodus.extract_datetime
* exodus.count

to sort and select (filter) correctly depending on the data type.

```
select invoices with amount > 1000.00 by date by time
```

## Using

No manual usage should be required since the extension will be loaded during exodus installation and operation.

```
create extension pgexodus;
```

An extremely quick sanity check/test is performed every time the extension is created.

Like all extensions:
- It needs to be created individually in every database.
- If created in the template1 database then all new databases will inherit the extension.

## Dependencies

To build the extensions you need to have:
- CMake version 3.10 or later
- PostgreSQL server version 10 or later.
- PostgreSQL server development files version 10 or later.

You can install these dependencies on Ubuntu (18.04 or later) using:

```
apt update
apt install cmake postgresql postgresql-server-dev-all
```

## Build and install

```./install.sh```

```
apt update
apt install cmake postgresql postgresql-server-dev-all

rm build -rf
mkdir build

cmake . -B build
cmake --build build
cmake --install build
```
## Uninstall

Remove the files listed in build/install_manifest.txt
