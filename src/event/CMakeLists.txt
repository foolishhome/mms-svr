CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(LIBEVENT_LIBNAME yylibevent)

PROJECT(${LIBEVENT_LIBNAME})
ADD_LIBRARY(${LIBEVENT_LIBNAME} SHARED
yylibevent.cpp
)

#--------------------
# Package look for
#--------------------
FIND_PACKAGE(LibEvent)
IF(LibEvent_FOUND)
	MESSAGE(STATUS "FOUND PACKAGE: ${LIBEVENT_INCLUDE_DIR} AND ${LIBEVENT_LIB}")
ELSE()
	MESSAGE(STATUS "! NOT FOUND PACKAGE: ${LIBEVENT_INCLUDE_DIR} OR ${LIBEVENT_LIB}")	
ENDIF(LibEvent_FOUND)

#--------------------
# build parameter
#--------------------
INCLUDE_DIRECTORIES(
../
../../
../../include
)

TARGET_LINK_LIBRARIES(${LIBEVENT_LIBNAME}
${LIBEVENT_LIB}
event_pthreads
)

