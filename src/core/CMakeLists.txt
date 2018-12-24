CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (PLPCore)

set (LP_VERSION_MAJOR 1)
set (LP_VERSION_MINOR 0)
set (LP_VERSION_PATCH 0)

set (CMAKE_CXX_STANDARD 11)

IF(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /WX /arch:AVX /Ox /Ob2 /Ot /Gt /Zc:implicitNoexcept-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /WX /arch:AVX /Od /Ob0 /Zc:implicitNoexcept- /DP3D_DEBUG")
ENDIF(MSVC)

# Configuration GUI goes here


# =====================================================================================

#                                     Global includes

# =====================================================================================

SET(HEADERS       
    FileSystemManager.h
    PLPCore.h
    Utils.h
    )

SET(SOURCES 
    FileSystemManager.cpp
    PLPCore.cpp
    Utils.cpp
    )


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

# GLFW
#SET(NLE_GLFW_INCLUDE_PATH
#    "" CACHE PATH "NLE GLFW include path")

#IF(NLE_GLFW_INCLUDE_PATH STREQUAL "")
#    MESSAGE(SEND_ERROR "NLE GLFW include path must be specified")
#ENDIF(NLE_GLFW_INCLUDE_PATH STREQUAL "")

#SET(NLE_GLFW_LIBRARY_DEBUG_PATH
#    "" CACHE PATH "NLE GLFW library debug path")

#IF(NLE_GLFW_LIBRARY_DEBUG_PATH STREQUAL "")
#    MESSAGE(SEND_ERROR "NLE GLFW library debug path must be specified")
#ENDIF(NLE_GLFW_LIBRARY_DEBUG_PATH STREQUAL "")

#SET(NLE_GLFW_LIBRARY_RELEASE_PATH
#    "" CACHE PATH "NLE GLFW library release path")

#IF(NLE_GLFW_LIBRARY_RELEASE_PATH STREQUAL "")
#    MESSAGE(SEND_ERROR "NLE GLFW library release path must be specified")
#ENDIF(NLE_GLFW_LIBRARY_RELEASE_PATH STREQUAL "")



SET(HEADERS
        ${HEADERS}
        #${NLE_IMGUI_INCLUDE_PATH}/imgui.h
        #${NLE_IMGUI_INCLUDE_PATH}/imconfig.h
        #${NLE_IMGUI_INCLUDE_PATH}/imgui_internal.h
        #${NLE_IMGUI_INCLUDE_PATH}/stb_rect_pack.h
        #${NLE_IMGUI_INCLUDE_PATH}/stb_textedit.h
        #${NLE_IMGUI_INCLUDE_PATH}/stb_truetype.h
        #p3d/device/d3d11/DDSTextureLoader.h
        #${P3D_DIRECT_X_TEX_INCLUDE_PATH}/WICTextureLoader/WICTextureLoader.h
    )

    SET(SOURCES 
        ${SOURCES}
        #${NLE_IMGUI_INCLUDE_PATH}/imgui.cpp
        #${NLE_IMGUI_INCLUDE_PATH}/imgui_demo.cpp
        #${NLE_IMGUI_INCLUDE_PATH}/imgui_draw.cpp
        #p3d/device/d3d11/DDSTextureLoader.cpp
        #${P3D_DIRECT_X_TEX_INCLUDE_PATH}/WICTextureLoader/WICTextureLoader.cpp
    )


INCLUDE_DIRECTORIES(
    ${LUA_INCLUDE_PATH}
    #${NLE_D3D11_INCLUDE_PATH}
    #${P3D_GLM_INCLUDE_PATH}
    )

ADD_DEFINITIONS(

    )

LINK_DIRECTORIES(
    ${LUA_LIBRARY_DEBUG_PATH}
    ${LUA_LIBRARY_RELEASE_PATH}
    #${NLE_D3D11_LIBRARY_DEBUG_PATH}
    #${NLE_D3D11_LIBRARY_RELEASE_PATH}
    )

add_library (PLPCORE STATIC
  ${HEADERS}
  ${SOURCES}
)

TARGET_LINK_LIBRARIES(PLPCORE 
    debug lua51
    optimized lua51

    #debug d3d11.lib
    #optimized d3d11.lib
    #debug d3dcompiler.lib
    #optimized d3dcompiler.lib
    )

set_target_properties(PLPCORE PROPERTIES DEBUG_POSTFIX _d)
