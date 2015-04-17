# - Find LibMysql (a cross event library)
# This module defines
# LIBMYSQL_INCLUDE_DIR, where to find LibMysql headers
# LIBMYSQL_LIB, LibMysql libraries
# LibMysql_FOUND, If false, do not try to use libmysql

set(LibMysql_EXTRA_PREFIXES /usr/local /opt/local /usr "$ENV{HOME}")
foreach(prefix ${LibMysql_EXTRA_PREFIXES})
	list(APPEND LibMysql_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibMysql_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBMYSQL_INCLUDE_DIR mysql.h PATHS ${LibMysql_INCLUDE_PATHS} PATH_SUFFIXES mysql)
find_library(LIBMYSQL_LIB NAMES mysqlclient PATHS ${LibMysql_LIB_PATHS})

if (LIBMYSQL_LIB AND LIBMYSQL_INCLUDE_DIR)
  set(LibMysql_FOUND TRUE)
  set(LIBMYSQL_LIB ${LIBMYSQL_LIB})
else ()
  set(LibMysql_FOUND FALSE)
endif ()

if (LibMysql_FOUND)
  if (NOT LibMysql_FIND_QUIETLY)
    message(STATUS "Found libmysql: ${LIBMYSQL_LIB}")
  endif ()
else ()
    if (LibMysql_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libmysql.")
    endif ()
    message(STATUS "libmysql NOT found.")
endif ()

mark_as_advanced(
    LIBMYSQL_LIB
    LIBMYSQL_INCLUDE_DIR
  )

