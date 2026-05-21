cmake_minimum_required(VERSION 3.15)

# Init

	message("--------------------")
	message("-- CMAKE PGEXODUS --")
	message("--------------------")

# Project

	project(pgexodus)
	set(EXTENSION_NAME    pgexodus)
	set(EXTENSION_VERSION 1.0)

# Custom FindPostgreSQL2.cmake - Why?

	list(
		APPEND
		CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
	)

# FindPostgreSQL2.cmake

	message("PGPATH=$ENV{PGPATH}")

	#https://cmake.org/cmake/help/latest/module/FindPostgreSQL.html
	find_package(PostgreSQL2 ${PGSQL_MIN_VERSION} REQUIRED)

# Configure the extension control file

	configure_file(
		pgexodus.control.in ${EXTENSION_NAME}.control @ONLY
	)

# Configure the sql create and alter extension script
# /usr/share/postgresql/18/extension/pgexodus--1.0.sql

	configure_file(
		install.sql.in ${EXTENSION_NAME}--${EXTENSION_VERSION}.sql @ONLY
	)

# Install the extension control file and sql script

	install(
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}.control"
			"${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}--${EXTENSION_VERSION}.sql"
		DESTINATION ${PostgreSQL_EXTENSION_DIR}
	)

# Testing

	enable_testing()

# Test - using pg_regress - with force disconnection of any prior hung regression tests

add_test(
    NAME pgexodus_test
    COMMAND
        bash -c "\
            PG_PORT=`grep '^\\s*port\\s*=\\s*\\([0-9]\\)*' /etc/postgresql/${PG_VER}/main/postgresql.conf|grep [0-9]* -o`;\
            ! sudo -u postgres psql --port $PG_PORT postgres -c 'select version()' ||\
            sudo -u postgres psql --port $PG_PORT postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.pid) FROM pg_stat_activity WHERE pg_stat_activity.datname = 'regression' AND pid <> pg_backend_pid();\" &&\
            sudo -u postgres\
            ${PostgreSQL_PACKAGE_LIBRARY_DIR}/pgxs/src/test/regress/pg_regress\
            --outputdir=/tmp\
            --load-extension=pgexodus\
            --port $PG_PORT\
        "
)
# Exit
