CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tinyap C)

SET(TINYAP_VERSION_MAJOR 2)
SET(TINYAP_VERSION_MINOR 0)
SET(TINYAP_VERSION_PATCH beta)

SET(TINYAP_VERSION "${TINYAP_VERSION_MAJOR}.${TINYAP_VERSION_MINOR}-${TINYAP_VERSION_PATCH}")

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
OPTION(USE_FOREST "Compute and output all possible parse trees, not just first one. Slower algorithm but useful for ambiguous grammars." ON)

ADD_SUBDIRECTORY(src)
#INCLUDE(CMakeCPack.cmake)
