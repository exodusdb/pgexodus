cmake_minimum_required(VERSION 3.15)

# Init

	message("--------------------")
	message("-- CMAKE PGEXODUS --")
	message("--------------------")

# Project

	project(pgexodus C)
	set(EXTENSION_NAME    pgexodus)
	set(EXTENSION_VERSION 1.0)

# C options

	set(CMAKE_CC_STANDARD 20)
#	if(NOT CMAKE_BUILD_TYPE)
#		set(
#			#CMAKE_BUILD_TYPE "RelWithDebInfo"
#			CMAKE_BUILD_TYPE "RELEASE"
#			CACHE STRING "Default build type: RelWithDebInfo"
#			FORCE
#		)
#	endif()
	set(CMAKE_BUILD_TYPE "RELEASE")

	message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
    message("CMAKE_C_FLAGS are  ${CMAKE_C_FLAGS}")

#    message("CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
#    message("CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
#    message("CMAKE_C_FLAGS_MINSIZEREL is ${CMAKE_C_FLAGS_MINSIZEREL}")
#    message("CMAKE_C_FLAGS_RELWITHDEBINFO is ${CMAKE_C_FLAGS_RELWITHDEBINFO}")

# Testing

#	include(CTest)
	enable_testing()

# Minimum PostgreSQL version

	set(PGSQL_MIN_VERSION "10.0.0")

# Custom FindPostgreSQL.cmake

	list(
		APPEND
		CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
	)

# FindPostgreSQL2.cmake

	message("PGPATH=$ENV{PGPATH}")

	#https://cmake.org/cmake/help/latest/module/FindPostgreSQL.html
	find_package(PostgreSQL2 ${PGSQL_MIN_VERSION} REQUIRED)
	#message("PostgreSQL_PACKAGE_LIBRARY_DIR = " ${PostgreSQL_PACKAGE_LIBRARY_DIR})
	#message("PostgreSQL_LIBRARIES           = " ${PostgreSQL_LIBRARIES})
	#message("PostgreSQL_EXTENSION_DIR       = " ${PostgreSQL_EXTENSION_DIR})
	#message("PostgreSQL_VERSION_STRING      = " ${PostgreSQL_VERSION_STRING})
#	message("-- PostgreSQL_INCLUDE_DIRS        = " ${PostgreSQL_INCLUDE_DIRS})
#	message("-- PostgreSQL_SERVER_INCLUDE_DIRS = " ${PostgreSQL_SERVER_INCLUDE_DIRS})
#	message("-- PostgreSQL_LIBRARY_DIRS        = " ${PostgreSQL_LIBRARY_DIRS})
#	message(STATUS "PG_VER ${PG_VER}")

	# Moved to src/pgexodus.h
#	add_compile_options(
#		-Wno-ignored-attributes
#		# clang 18 warning
#		#warning: 'format' attribute argument not supported: gnu_printf [-Wignored-attributes]
#	)

# Create shared lib

	add_library(
		${EXTENSION_NAME}
		MODULE
		src/pgexodus.h
		src/pgexodus.c
		src/extract2.c
		src/extract_text.c
		src/extract_date.c
		src/extract_time.c
		src/extract_datetime.c
		src/extract_number.c
		src/count.c
	)

	target_compile_options(${EXTENSION_NAME} PUBLIC -Wno-unsafe-buffer-usage -Wno-reserved-identifier)

# lib suffix

	set_target_properties(
		${EXTENSION_NAME}
		PROPERTIES
		PREFIX ""
		SUFFIX ".so"
	)

# Source files

	set(
		SOURCES
	)

# Include dirs

	target_include_directories(
		${EXTENSION_NAME}
		PRIVATE
		${PostgreSQL_SERVER_INCLUDE_DIRS}
		#${PostgreSQL_INCLUDE_DIRS}
	)

## Include dirs
#
#	include_directories(
#		${PostgreSQL_INCLUDE_DIRS}
#		${PostgreSQL_SERVER_INCLUDE_DIRS}
#	)
#
## Link libs
#
#	target_link_libraries(
#		${EXTENSION_NAME}
#		PRIVATE
#		${PostgreSQL_LIBRARIES}
#	)
#
# Compile flags
#
#	target_compile_options(
#		${EXTENSION_NAME}
#		PRIVATE
#		-Wall -Werror -Wextra -O3
#		${PostgreSQL_LIBRARY_DIRS}
#	)

# Configure the control file

	configure_file(
		pgexodus.control.in ${EXTENSION_NAME}.control @ONLY
	)

# Configure the sql script

	configure_file(
		install.sql.in ${EXTENSION_NAME}--${EXTENSION_VERSION}.sql @ONLY
	)

# Install

	install(
		TARGETS ${EXTENSION_NAME}
		LIBRARY DESTINATION "${PostgreSQL_PACKAGE_LIBRARY_DIR}"
	)

# Install the control file and sql script

	install(
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}.control"
			"${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}--${EXTENSION_VERSION}.sql"
		DESTINATION ${PostgreSQL_EXTENSION_DIR}
	)

# Test - using pg_regress


	#Skip test if cannot sudo postgres psql on the right port
	# sudo -u postgres psql --port $PG_PORT postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.pid) FROM pg_stat_activity WHERE pg_stat_activity.datname = 'regression' AND pid <> pg_backend_pid();\" &&\
#	add_test(
#		NAME pgexodus_test
#		COMMAND
#            bash -c "\
#                PG_PORT=`grep '^\\s*port\\s*=\\s*\\([0-9]\\)*' /etc/postgresql/${PG_VER}/main/postgresql.conf|grep [0-9]* -o`;\
#                ! sudo -u postgres psql --port $PG_PORT postgres -c 'select version()' ||\
#                sudo -u postgres\
#                ${PostgreSQL_PACKAGE_LIBRARY_DIR}/pgxs/src/test/regress/pg_regress\
#                --outputdir=/tmp\
#                --load-extension=pgexodus\
#                --port $PG_PORT\
#            "
#	)

# test with disconnection inserted
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

#	message("--PGEXODUS CMAKE EXIT --")

#	/usr/bin/cc
#		-Dpgexodus_EXPORTS
#		-I/usr/include/postgresql
#		-I/usr/include/postgresql/14/server
#		-O3 -DNDEBUG -fPIC -MD
#		-MT CMakeFiles/pgexodus.dir/src
#		/pgexodus.c.o
#		-MF CMakeFiles/pgexodus.dir/src/pgexodus.c.o.d
#		-o CMakeFiles/pgexodus.dir/src/pgexodus.c.o
#		-c /root/exodus/exodus/pgexodus/src/pgexodus.c

