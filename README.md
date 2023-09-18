# pgexodus PostgreSQL Extension

## Dependencies

To build the extensions you need to have:
- CMake version 3.10 or later
- PostgreSQL server version 10 or later.
- PostgreSQL server development files version 10 or later.

You can install these dependencies on Ubuntu (18.04 or later) using:

```
apt install cmake postgresql postgresql-server-dev-1*
```

## Building

To build the extensions for installation using the default prefix
(`/usr/local`) and keep the build files separate from the source
files:

```
mkdir build
cd build
cmake ..
cmake --build ..
```

If you want to use a different version of Postgres than the default,
you can set `PGPATH` to point to the prefix where you have installed
PostgreSQL.

```
cmake .. -DPGPATH=/usr/local/pgsql/15
```