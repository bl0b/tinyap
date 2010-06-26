CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tinyap C)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)

SET(fchk malloc realloc stat atexit memset regcomp strchr strdup strndup)

FOREACH(f ${fchk})
	STRING(TOUPPER ${f} F)
	MESSAGE(STATUS "${f} -> ${F}")
	CHECK_FUNCTION_EXISTS(${f} HAVE_${F})
ENDFOREACH(f)

CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_LIBDL)

CONFIGURE_FILE(config.h.cmake.in ${CMAKE_BINARY_DIR}/src/config.h)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/src/config.h PROPERTIES GENERATED ON)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src)

OPTION(TINYAP_SAFEMODE "Safe mode : check type before accessing any node in the AST" ON)
OPTION(NODE_CACHE_STATS "Compute and output statistics about node cache usage." OFF)

ADD_SUBDIRECTORY(src)
#INCLUDE(CMakeCPack.cmake)
