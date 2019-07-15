#[[
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
]]

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (LCCore)

set (LC_VERSION_MAJOR 1)
set (LC_VERSION_MINOR 0)
set (LC_VERSION_PATCH 0)

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
    CircularLineBuffer.h
    Core.h
    CoreI.h
    FileLock.h
    FileReader.h
    FileReaderI.h
    FileWriter.h
    FileWriterI.h
    FrameBuffer.h
    FStreamPagedReader.h
    FStreamPagedWriter.h
    GenFileTracker.h
    IndexedLineReader.h
    IndexReader.h
    IndexReaderI.h
    IndexWriter.h
    IndexWriterI.h
    LineBuffer.h
    LineReader.h
    Logger.h
    MemMappedPagedReader.h
    PagedReader.h
    PagedWriter.h
    ProgressReporter.h
    ReturnType.h
    Scanner.h
    TaskRunner.h
    TextComparator.h
    Thread.h
    Timer.h
    Utils.h
    )

SET(SOURCES 
    CircularLineBuffer.cpp
    Core.cpp
    FileLock.cpp
    FileReader.cpp
    FileWriter.cpp
    FrameBuffer.cpp
    FStreamPagedReader.cpp
    FStreamPagedWriter.cpp
    GenFileTracker.cpp
    IndexedLineReader.cpp
    IndexReader.cpp
    IndexWriter.cpp
    LineBuffer.cpp
    LineReader.cpp
    Logger.cpp
    MemMappedPagedReader.cpp
    ProgressReporter.cpp
    Scanner.cpp
    Thread.cpp
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


#lua-intf
SET(LUA_INTF_INCLUDE_PATH
	"" CACHE PATH "LUA_INTF include path")

IF(LUA_INTF_INCLUDE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "LUA_INTF include path must be specified")
ENDIF(LUA_INTF_INCLUDE_PATH STREQUAL "")


#C++ Cereal
SET(CEREAL_INCLUDE_PATH
	"" CACHE PATH "Cereal include path")

IF(CEREAL_INCLUDE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "Cereal include path must be specified")
ENDIF(CEREAL_INCLUDE_PATH STREQUAL "")


# TBB
SET(TBB_INCLUDE_PATH
	"" CACHE PATH "TBB include path")

IF(TBB_INCLUDE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "TBB include path must be specified")
ENDIF(TBB_INCLUDE_PATH STREQUAL "")

SET(TBB_LIBRARY_DEBUG_PATH
	"" CACHE PATH "TBB library debug path")

IF(TBB_LIBRARY_DEBUG_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "TBB library debug path must be specified")
ENDIF(TBB_LIBRARY_DEBUG_PATH STREQUAL "")

SET(TBB_LIBRARY_RELEASE_PATH
	"" CACHE PATH "TBB library release path")

IF(TBB_LIBRARY_RELEASE_PATH STREQUAL "")
	MESSAGE(SEND_ERROR "TBB library release path must be specified")
ENDIF(TBB_LIBRARY_RELEASE_PATH STREQUAL "")


INCLUDE_DIRECTORIES(
    ${LUA_INCLUDE_PATH}
    ${LUA_INTF_INCLUDE_PATH}
    ${TBB_INCLUDE_PATH}
    ${CEREAL_INCLUDE_PATH}
    )

ADD_DEFINITIONS(
    -DPLP_EXPORT
    )

LINK_DIRECTORIES(
    ${LUA_LIBRARY_DEBUG_PATH}
    ${LUA_LIBRARY_RELEASE_PATH}
    ${TBB_LIBRARY_DEBUG_PATH}
    ${TBB_LIBRARY_RELEASE_PATH}
    )

add_library (lcCore SHARED
  ${HEADERS}
  ${SOURCES}
)

TARGET_LINK_LIBRARIES(lcCore 
    debug lua51
    optimized lua51

    debug tbb_debug
	optimized tbb
    )

set_target_properties(lcCore PROPERTIES DEBUG_POSTFIX _d)
