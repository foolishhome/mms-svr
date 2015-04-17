# - Find LibJson (a cross event library)
# This module defines
# LIBJSON_INCLUDE_DIR, where to find LibJson headers
# LIBJSON_LIB, LibJson libraries
# LibJson_FOUND, If false, do not try to use libjson

set(LibJson_EXTRA_PREFIXES /usr/local /opt/local /usr "$ENV{HOME}")
foreach(prefix ${LibJson_EXTRA_PREFIXES})
	list(APPEND LibJson_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibJson_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBJSON_INCLUDE_DIR json.h PATHS ${LibJson_INCLUDE_PATHS} PATH_SUFFIXES json)
find_library(LIBJSON_LIB NAMES json PATHS ${LibJson_LIB_PATHS})

if (LIBJSON_LIB AND LIBJSON_INCLUDE_DIR)
  set(LibJson_FOUND TRUE)
  set(LIBJSON_LIB ${LIBJSON_LIB})
else ()
  set(LibJson_FOUND FALSE)
endif ()

if (LibJson_FOUND)
  if (NOT LibJson_FIND_QUIETLY)
    message(STATUS "Found libjson: ${LIBJSON_LIB}")
  endif ()
else ()
    if (LibJson_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libjson.")
    endif ()
    message(STATUS "libjson NOT found.")
endif ()

mark_as_advanced(
    LIBJSON_LIB
    LIBJSON_INCLUDE_DIR
  )

