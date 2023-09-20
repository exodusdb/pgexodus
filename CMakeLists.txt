cmake_minimum_required(VERSION 3.0)
project(pgexodus C)

include(CTest)
enable_testing()

# Set the minimum required PostgreSQL version
set(PGSQL_MIN_VERSION "10.0.0")

# Find PostgreSQL
#https://cmake.org/cmake/help/latest/module/FindPostgreSQL.html
find_package(PostgreSQL ${PGSQL_MIN_VERSION} REQUIRED)
message("INCLUDEDIR-SERVER           = " ${INCLUDEDIR-SERVER})
message("PostgreSQL_LIBRARIES        = " ${PostgreSQL_LIBRARIES})
message("PostgreSQL_INCLUDE_DIRS     = " ${PostgreSQL_INCLUDE_DIRS})
message("PostgreSQL_LIBRARY_DIRS     = " ${PostgreSQL_LIBRARY_DIRS})
message("PostgreSQL_VERSION_STRING   = " ${PostgreSQL_VERSION_STRING})
message("PostgreSQL_TYPE_INCLUDE_DIR = " ${PostgreSQL_TYPE_INCLUDE_DIR})

# Find PostgreSQL include dir
execute_process(
	COMMAND pg_config --includedir-server
	OUTPUT_VARIABLE _pg_server_inc_dirs
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find PostgreSQL package lib dir
execute_process(
	COMMAND pg_config --pkglibdir
	OUTPUT_VARIABLE _pg_pkglibdir
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find PostgreSQL share dir
execute_process(
	COMMAND pg_config --sharedir
	OUTPUT_VARIABLE _pg_sharedir
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set the extension source files
set(SOURCES
	src/pgexodus.h
	src/pgexodus.c
	src/extract.c
	src/extract_text.c
	src/extract_date.c
	src/extract_time.c
	src/extract_datetime.c
	src/extract_number.c
	src/count.c
)

# Set the shared library name and version
set(EXTENSION_NAME pgexodus)
set(EXTENSION_VERSION 1.0)

# Create the shared library for the extension
add_library(${EXTENSION_NAME} MODULE ${SOURCES})

# Include PostgreSQL headers
target_include_directories(${EXTENSION_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS} ${_pg_server_inc_dirs})

# Link against PostgreSQL libraries
target_link_libraries(${EXTENSION_NAME} PRIVATE ${PostgreSQL_LIBRARIES})

# Set the required compilation flags
target_compile_options(${EXTENSION_NAME} PRIVATE -Wall -Werror -Wextra -O3)

# Set the target properties
set_target_properties(${EXTENSION_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Create the control file
configure_file(pgexodus.control.in ${EXTENSION_NAME}.control @ONLY)

# Create the install script
configure_file(install.sql.in ${EXTENSION_NAME}--${EXTENSION_VERSION}.sql @ONLY)

# Install the extension lib
install(TARGETS ${EXTENSION_NAME}
    #LIBRARY DESTINATION "${PGSQL_PKGLIBDIR}"
    LIBRARY DESTINATION "${_pg_pkglibdir}"
)

# Install the control files
install(FILES
    "${CMAKE_BINARY_DIR}/${EXTENSION_NAME}.control"
    "${CMAKE_BINARY_DIR}/${EXTENSION_NAME}--${EXTENSION_VERSION}.sql"
    DESTINATION "${_pg_sharedir}/extension"
)

add_test(NAME pgexodus_test COMMAND sudo -u postgres psql postgres -c "drop extension if exists pgexodus;create extension pgexodus")
