# - Find LibSSL (a cross event library)
# This module defines
# LIBSSL_INCLUDE_DIR, where to find LibSSL headers
# LIBSSL_LIB, LibSSL libraries
# LibSSL_FOUND, If false, do not try to use libssl

set(LibSSL_EXTRA_PREFIXES /usr/local/ssl /usr/local /opt/local /usr "$ENV{HOME}")
foreach(prefix ${LibSSL_EXTRA_PREFIXES})
	list(APPEND LibSSL_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibSSL_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBSSL_INCLUDE_DIR ssl.h PATHS ${LibSSL_INCLUDE_PATHS} PATH_SUFFIXES openssl)
find_library(LIBSSL_LIB NAMES ssl PATHS ${LibSSL_LIB_PATHS})
find_library(LIBSSL_LIB_CRYPT NAMES crypto PATHS ${LibSSL_LIB_PATHS})

if (LIBSSL_LIB AND LIBSSL_INCLUDE_DIR)
  set(LibSSL_FOUND TRUE)
  set(LIBSSL_LIB ${LIBSSL_LIB})
else ()
  set(LibSSL_FOUND FALSE)
endif ()

if (LibSSL_FOUND)
  if (NOT LibSSL_FIND_QUIETLY)
    message(STATUS "Found libssl: ${LIBSSL_LIB}")
  endif ()
else ()
    if (LibSSL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libssl.")
    endif ()
    message(STATUS "libssl NOT found.")
endif ()

set(pos -1)
string(LENGTH "${LIBSSL_INCLUDE_DIR}" pos)
set(subs "")
math(EXPR pos "${pos}-8")
string(SUBSTRING "${LIBSSL_INCLUDE_DIR}" 0 ${pos} subs)
set(LIBSSL_INCLUDE_DIR ${subs})

mark_as_advanced(
    LIBSSL_LIB
    LIBSSL_LIB_CRYPT
    LIBSSL_INCLUDE_DIR
  )
