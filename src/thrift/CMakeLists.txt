
SET(THIRFT_LIBNAME yythirft)

PROJECT(${THIRFT_LIBNAME})
ADD_LIBRARY(${THIRFT_LIBNAME}
yythirft.cpp
)

#--------------------
# Package look for
#--------------------
FIND_PACKAGE(LibThirft)
IF(LibThirft_FOUND)
	MESSAGE(STATUS "FOUND PACKAGE: ${LIBTHIRFT_INCLUDE_DIR} AND ${LIBTHIRFT_LIB}")
ELSE()
	MESSAGE(STATUS "! NOT FOUND PACKAGE: ${LIBTHIRFT_INCLUDE_DIR} OR ${LIBTHIRFT_LIB}")	
ENDIF(LibThirft_FOUND)

