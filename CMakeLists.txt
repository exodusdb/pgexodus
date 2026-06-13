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
		pgexodus.sql ${EXTENSION_NAME}--${EXTENSION_VERSION}.sql @ONLY
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
        bash -c "
            set -euxo pipefail

            : 'Extract PostgreSQL port'
            PG_PORT=\$(grep '^\\s*port\\s*=\\s*[0-9]' /etc/postgresql/\${PG_VER}/main/postgresql.conf | grep -o '[0-9]\\+')

            : 'Cleanup function'
            cleanup() {
                : '=== Cleaning up xo_regression database ==='
                sudo -u postgres psql --port \"\$PG_PORT\" postgres -c '
                    -- Terminate existing connections
                    SELECT pg_terminate_backend(pid)
                    FROM pg_stat_activity
                    WHERE datname = '\''xo_regression'\''
                      AND pid <> pg_backend_pid();
                ' 2>/dev/null || true

                sudo -u postgres psql --port \"\$PG_PORT\" postgres -c '
                    DROP DATABASE IF EXISTS xo_regression;
                ' 2>/dev/null || true

                : '=== Cleanup completed ==='
                sleep 1
            }

            : 'Always clean before starting'
            cleanup

            : 'Ensure cleanup also runs on exit (success or failure)'
            trap cleanup EXIT

            : '=== Starting pg_regress test ==='

            : 'Run the regression test'
            sudo -u postgres \
                ${PostgreSQL_PACKAGE_LIBRARY_DIR}/pgxs/src/test/regress/pg_regress \
                --outputdir=/tmp \
                --load-extension=plperl \
                --load-extension=pgexodus \
                --port \"\$PG_PORT\" \
                --dbname=xo_regression \
                \"\$@\"

            : '=== Test completed successfully ==='
        "
)
# Exit
