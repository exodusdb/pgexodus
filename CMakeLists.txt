cmake_minimum_required(VERSION 3.1)
project(extension C)

message("")
message("--EXODUS PGEXODUS CMAKE INIT --")

#message("CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")
#message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
#message("CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PostgreSQL REQUIRED)

#set(CMAKE_CXX_STANDARD 14)

include_directories(${PostgreSQL_INCLUDE_DIRS}
                    ${PostgreSQL_SERVER_INCLUDE_DIRS})

enable_testing()
add_subdirectory(src)

#add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

message("--EXODUS PGEXODUS CMAKE EXIT --")
