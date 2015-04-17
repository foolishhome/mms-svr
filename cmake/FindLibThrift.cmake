# - Find LibThrift (a cross event library)
# This module defines
# LIBTHRIFT_INCLUDE_DIR, where to find LibThrift headers
# LIBTHRIFT_LIB, LibThrift libraries
# LibThrift_FOUND, If false, do not try to use libredis

set(LibThrift_EXTRA_PREFIXES /usr/local /opt/local /usr "$ENV{HOME}")
foreach(prefix ${LibThrift_EXTRA_PREFIXES})
	list(APPEND LibThrift_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibThrift_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBTHRIFT_INCLUDE_DIR hiredis.h PATHS ${LibThrift_INCLUDE_PATHS} PATH_SUFFIXES hiredis)
find_library(LIBTHRIFT_LIB NAMES hiredis PATHS ${LibThrift_LIB_PATHS})

if (LIBTHRIFT_LIB AND LIBTHRIFT_INCLUDE_DIR)
  set(LibThrift_FOUND TRUE)
  set(LIBTHRIFT_LIB ${LIBTHRIFT_LIB})
else ()
  set(LibThrift_FOUND FALSE)
endif ()

if (LibThrift_FOUND)
  if (NOT LibThrift_FIND_QUIETLY)
    message(STATUS "Found libredis: ${LIBTHRIFT_LIB}")
  endif ()
else ()
    if (LibThrift_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libredis.")
    endif ()
    message(STATUS "libredis NOT found.")
endif ()

mark_as_advanced(
    LIBTHRIFT_LIB
    LIBTHRIFT_INCLUDE_DIR
  )

