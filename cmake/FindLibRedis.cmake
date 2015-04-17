# - Find LibRedis (a cross event library)
# This module defines
# LIBREDIS_INCLUDE_DIR, where to find LibRedis headers
# LIBREDIS_LIB, LibRedis libraries
# LibRedis_FOUND, If false, do not try to use libredis

set(LibRedis_EXTRA_PREFIXES /usr/local /opt/local /usr "$ENV{HOME}")
foreach(prefix ${LibRedis_EXTRA_PREFIXES})
	list(APPEND LibRedis_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibRedis_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBREDIS_INCLUDE_DIR hiredis.h PATHS ${LibRedis_INCLUDE_PATHS} PATH_SUFFIXES hiredis)
find_library(LIBREDIS_LIB NAMES hiredis PATHS ${LibRedis_LIB_PATHS})

if (LIBREDIS_LIB AND LIBREDIS_INCLUDE_DIR)
  set(LibRedis_FOUND TRUE)
  set(LIBREDIS_LIB ${LIBREDIS_LIB})
else ()
  set(LibRedis_FOUND FALSE)
endif ()

if (LibRedis_FOUND)
  if (NOT LibRedis_FIND_QUIETLY)
    message(STATUS "Found libredis: ${LIBREDIS_LIB}")
  endif ()
else ()
    if (LibRedis_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libredis.")
    endif ()
    message(STATUS "libredis NOT found.")
endif ()

mark_as_advanced(
    LIBREDIS_LIB
    LIBREDIS_INCLUDE_DIR
  )

