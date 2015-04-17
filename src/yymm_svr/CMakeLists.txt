CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#--------------------
# Project Manager
#--------------------
PROJECT(${EXE_NAME})
ADD_EXECUTABLE(${EXE_NAME}
yymms_svr.cpp
main.cpp
cfg.cpp
cmdproc.cpp
mmsexecmag.cpp
)

EXECUTE_PROCESS(COMMAND echo "begin to create yymmssvr server in ${CMAKE_SOURCE_DIR}"
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


ADD_DEPENDENCIES(${EXE_NAME} event)

#--------------------
# Package look for
#--------------------
FIND_PACKAGE(LibJson)
IF(LibJson_FOUND)
	MESSAGE(STATUS "FOUND PACKAGE: ${LIBJSON_INCLUDE_DIR} AND ${LIBJSON_LIB}")
ELSE()
	MESSAGE(STATUS "! NOT FOUND PACKAGE: ${LIBJSON_INCLUDE_DIR} OR ${LIBJSON_LIB}")	
ENDIF(LibJson_FOUND)

FIND_PACKAGE(LibSSL)
IF(LibSSL_FOUND)
	MESSAGE(STATUS "FOUND PACKAGE: ${LIBSSL_INCLUDE_DIR} AND ${LIBSSL_LIB}")
ELSE()
	MESSAGE(STATUS "! NOT FOUND PACKAGE: ${LIBSSL_INCLUDE_DIR} OR ${LIBSSL_LIB}")	
ENDIF(LibSSL_FOUND)

#--------------------
# build parameter
#--------------------

SET(CMAKE_LIBRARY_PATH /usr/local/lib /usr/lib)
SET(CMAKE_INCLUDE_PATH /usr/local/include /usr/include)

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)


INCLUDE_DIRECTORIES(
${PROJECT_SOURCE_DIR}
../
../../
../../include
${LIBREDIS_INCLUDE_DIR}
${LIBMYSQL_INCLUDE_DIR}
${LIBJSON_INCLUDE_DIR}
${LIBSSL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(${EXE_NAME}
dl
pthread
${CMAKE_SOURCE_DIR}/lib/libyylibevent.so
${CMAKE_SOURCE_DIR}/lib/libyyredis.so
${CMAKE_SOURCE_DIR}/lib/libyymysql.so
${LIBREDIS_LIB}
${LIBJSON_LIB}
${LIBSSL_LIB}
${LIBSSL_LIB_CRYPT}
yyxml
)

