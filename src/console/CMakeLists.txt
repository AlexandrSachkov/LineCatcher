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


# PLP core
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

SET(PLPCORE_LIBRARY_RELEASE_PATH
    "" CACHE PATH "PLP Core library release path")

IF(PLPCORE_LIBRARY_RELEASE_PATH STREQUAL "")
    MESSAGE(SEND_ERROR "PLP Core library release path must be specified")
ENDIF(PLPCORE_LIBRARY_RELEASE_PATH STREQUAL "")



INCLUDE_DIRECTORIES(
    ${PLPCORE_INCLUDE_PATH}
    )

ADD_DEFINITIONS(

    )

LINK_DIRECTORIES(
    ${PLPCORE_LIBRARY_DEBUG_PATH}
    ${PLPCORE_LIBRARY_RELEASE_PATH}
    )

set(LIBS 
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