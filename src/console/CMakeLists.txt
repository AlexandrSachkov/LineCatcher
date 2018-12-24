CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (PLPConsole)

set (PLP_VERSION_MAJOR 1)
set (PLP_VERSION_MINOR 0)
set (PLP_VERSION_PATCH 0)

set (CMAKE_CXX_STANDARD 11)

IF(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /WX /arch:AVX /Ox /Ob2 /Ot /Gt /Zc:implicitNoexcept-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /WX /arch:AVX /Od /Ob0 /Zc:implicitNoexcept- /DP3D_DEBUG")
ENDIF(MSVC)

# Configuration GUI goes here


# =====================================================================================

#                                     Global includes

# =====================================================================================

# Lua
SET(LUA_INCLUDE_PATH
	"" CACHE PATH "Lua include path")

IF(LUA_INCLUDE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "Lua include path must be specified")
ENDIF(LUA_INCLUDE_PATH STREQUAL "")

SET(LUA_LIBRARY_DEBUG_PATH
	"" CACHE PATH "Lua library debug path")

IF(LUA_LIBRARY_DEBUG_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "Lua library debug path must be specified")
ENDIF(LUA_LIBRARY_DEBUG_PATH STREQUAL "")

SET(LUA_LIBRARY_RELEASE_PATH
	"" CACHE PATH "Lua library release path")

IF(LUA_LIBRARY_RELEASE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "Lua library release path must be specified")
ENDIF(LUA_LIBRARY_RELEASE_PATH STREQUAL "")

# P3D
SET(PLPCORE_INCLUDE_PATH
    "" CACHE PATH "PLP Core include path")

IF(PLPCORE_INCLUDE_PATH STREQUAL "")
    MESSAGE(SEND_ERROR "PLP Core include path must be specified")
ENDIF(PLPCORE_INCLUDE_PATH STREQUAL "")

SET(PLPCORE_LIBRARY_DEBUG_PATH
    "" CACHE PATH "PLP Core library debug path")

IF(PLPCORE_LIBRARY_DEBUG_PATH STREQUAL "")
    MESSAGE(SEND_ERROR "PLP Core library debug path must be specified")
ENDIF(PLPCORE_LIBRARY_DEBUG_PATH STREQUAL "")

SET(PLP_CORE_LIBRARY_RELEASE_PATH
    "" CACHE PATH "PLP Core library release path")

IF(PLP_CORE_LIBRARY_RELEASE_PATH STREQUAL "")
    MESSAGE(SEND_ERROR "PLP Core library release path must be specified")
ENDIF(PLP_CORE_LIBRARY_RELEASE_PATH STREQUAL "")



INCLUDE_DIRECTORIES(
    ${LUA_INCLUDE_PATH}
    ${PLPCORE_INCLUDE_PATH}
    )

ADD_DEFINITIONS(

    )

LINK_DIRECTORIES(
    ${LUA_LIBRARY_DEBUG_PATH}
    ${LUA_LIBRARY_RELEASE_PATH}
    ${PLPCORE_LIBRARY_DEBUG_PATH}
    ${PLP_CORE_LIBRARY_RELEASE_PATH}
    )

set(LIBS 
    debug lua51
    optimized lua51
    
    debug PLPCORE_d.lib
    optimized PLPCORE.lib
)


add_executable (PLPConsole
  PLPConsole.cpp
)
TARGET_LINK_LIBRARIES(PLPConsole ${LIBS})

set_target_properties(
    PLPConsole

    PROPERTIES DEBUG_POSTFIX _d
)