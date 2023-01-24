 # CMakeList.txt
 #
 # Copyright (c) 2023 Mobileye
 #
 # Licensed under the Apache License, Version 2.0 (the License);
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an AS IS BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 
cmake_minimum_required(VERSION 3.5.1)

project(TTLConform${CONFORMANCE_SUFFIX})

include(GNUInstallDirs)

set( CONFORMANCE_SUFFIX "" )

if (NOT  CLConform_SOURCE_DIR)
    message(FATAL_ERROR "Define CMAKE_CURRENT_SOURCE_DIR as a path to OpenCL-CTS")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DCL_TARGET_OPENCL_VERSION=300)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_2_2_APIS=1)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_2_1_APIS=1)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_2_0_APIS=1)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_2_APIS=1)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_1_APIS=1)
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_0_APIS=1)

option(USE_CL_EXPERIMENTAL "Use Experimental definitions" OFF)
if(USE_CL_EXPERIMENTAL)
  add_definitions(-DCL_EXPERIMENTAL)
endif(USE_CL_EXPERIMENTAL)

#-----------------------------------------------------------
# Default Configurable Test Set
#-----------------------------------------------------------
option(D3D10_IS_SUPPORTED "Run DirectX 10 interop tests" OFF)
option(D3D11_IS_SUPPORTED "Run DirectX 11 interop tests" OFF)
option(GL_IS_SUPPORTED "Run OpenGL interop tests" OFF)
option(GLES_IS_SUPPORTED "Run OpenGL ES interop tests" OFF)


#-----------------------------------------------------------
# Tests prefix and suffix
#-----------------------------------------------------------
# Set the prefix and suffix for the generated executables
# For example, if you want the api executable to be test_conformance_api_12
# Set prefix to "test_conformance_" and suffix to "_12"
set(CONFORMANCE_PREFIX "test_" )
set(CONFORMANCE_SUFFIX "" )

#-----------------------------------------------------------
# Vendor Customization
#-----------------------------------------------------------
#Vendor Customization File can be included here to provide a way to automatically
#build driver as a dependency of the conformance tests, or other such CMake customization
include(CMakeVendor.txt OPTIONAL)

# CL_INCLUDE_DIR - path to dir with OpenCL headers
if(CL_INCLUDE_DIR AND CL_LIB_DIR)
    link_directories(${CL_LIB_DIR})
else(CL_INCLUDE_DIR AND CL_LIB_DIR)
    message(STATUS "OpenCL hasn't been found!")
    message(FATAL_ERROR "Either install OpenCL or pass -DCL_INCLUDE_DIR and -DCL_LIB_DIR")
endif(CL_INCLUDE_DIR AND CL_LIB_DIR)

# CLConform_GL_LIBRARIES_DIR - path to OpenGL libraries
if(GL_IS_SUPPORTED AND CLConform_GL_LIBRARIES_DIR)
    link_directories(${CLConform_GL_LIBRARIES_DIR})
endif (GL_IS_SUPPORTED AND CLConform_GL_LIBRARIES_DIR)

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
    set(CLConform_TARGET_ARCH ARM)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*|arm64.*|ARM64.*)")
    set(CLConform_TARGET_ARCH ARM64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(CLConform_TARGET_ARCH x86_64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*")
    set(CLConform_TARGET_ARCH x86)
endif()

if(NOT DEFINED CLConform_TARGET_ARCH)
    message (FATAL_ERROR "Target architecture not recognised. Exiting.")
endif()

macro(add_cxx_flag_if_supported flag)
    string(REGEX REPLACE "[-=+]" "" FLAG_NO_SIGNS ${flag})
    check_cxx_compiler_flag(${flag} COMPILER_SUPPORTS_${FLAG_NO_SIGNS})
    if(COMPILER_SUPPORTS_${FLAG_NO_SIGNS})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endmacro(add_cxx_flag_if_supported)

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?Clang")
    add_cxx_flag_if_supported(-Wmisleading-indentation)
    add_cxx_flag_if_supported(-Wno-narrowing)
    add_cxx_flag_if_supported(-Wno-format)
    add_cxx_flag_if_supported(-Werror)
    add_cxx_flag_if_supported(-Wno-error=cpp) # Allow #warning directive
    add_cxx_flag_if_supported(-Wno-error=unknown-pragmas) # Issue #785
    add_cxx_flag_if_supported(-Wno-error=asm-operand-widths) # Issue #784

    # -msse -mfpmath=sse to force gcc to use sse for float math,
    # avoiding excess precision problems that cause tests like int2float
    # to falsely fail. -ffloat-store also works, but WG suggested
    # that sse would be better.
    if(${CLConform_TARGET_ARCH} STREQUAL "x86_64" OR ${CLConform_TARGET_ARCH}
            STREQUAL "x86")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -mfpmath=sse")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2 -mfpmath=sse")

        add_cxx_flag_if_supported(-frounding-math)
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D__SSE__")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__SSE__")
endif()

if(DEFINED TTL_INSTALL_DIR)
    if(NOT IS_ABSOLUTE ${TTL_INSTALL_DIR})
        set(TTL_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/${TTL_INSTALL_DIR}")
    endif()
else()
    set(TTL_INSTALL_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/TTL")
endif()

set(TTL_INSTALL_DIR "${TTL_INSTALL_DIR}" CACHE PATH "The install location for the TTL files.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTTL_INSTALL_DIR=${TTL_INSTALL_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTTL_INSTALL_DIR=${TTL_INSTALL_DIR}")

if(MSVC)
    # Don't warn when using standard non-secure functions.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Don't warn about using the portable "strdup" function.
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
    # Fix std::min and std::max handling with windows.harness.
    add_compile_definitions(NOMINMAX)
endif()

if( WIN32 AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qlong-double -Qpc80 /DWIN32 /D_WINDOWS /W3 /GR /EHsc -nologo -Od -D_CRT_NONSTDC_NO_WARNINGS -EHsc -Wall -Qdiag-disable:68,111,177,186,161,869,1028,2259,2553,181,239,265,1188 -fp:strict -fp:source")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qlong-double -Qpc80 /DWIN32 /D_WINDOWS /W3 /GR /EHsc -nologo -Od -D_CRT_NONSTDC_NO_WARNINGS -EHsc -Wall -Qdiag-disable:68,111,177,186,161,869,1028,2259,2553,181,239,265,1188 -fp:strict -fp:source")
endif()

list(APPEND CLConform_LIBRARIES ${OPENCL_LIBRARIES})
if(ANDROID)
    list(APPEND CLConform_LIBRARIES m)
endif()
if(NOT DEFINED LINK_PTHREAD)
    if(ANDROID OR WIN32)
        set(LINK_PTHREAD OFF)
    else()
        set(LINK_PTHREAD ON)
    endif()
endif()
if(LINK_PTHREAD)
    list(APPEND CLConform_LIBRARIES pthread)
endif()

if(APPLE)
    find_library(corefoundation CoreFoundation)
    find_library(iokit IOKit)
    list(APPEND CLConform_LIBRARIES ${corefoundation})
    list(APPEND CLConform_LIBRARIES ${iokit})
endif(APPLE)

include_directories(SYSTEM ${CL_INCLUDE_DIR})
include_directories(${CLConform_SOURCE_DIR}/test_common/harness
                    ${CLConform_SOURCE_DIR}/test_common/gles
                    ${CLConform_SOURCE_DIR}/test_common/gl
                    ${CLConform_SOURCE_DIR}/test_common)

#add_subdirectory(test_common)
add_subdirectory(${CLConform_SOURCE_DIR}/test_common ${CMAKE_CURRENT_SOURCE_DIR}/build/test_common)

set(MODULE_NAME TTL)

set(${MODULE_NAME}_SOURCES
    main.cpp
    test_ttl.cpp
)

if(APPLE)
    list(APPEND ${MODULE_NAME}_SOURCES test_queue_priority.cpp)
endif(APPLE)

string(TOLOWER ${MODULE_NAME} MODULE_NAME_LOWER)

set(${MODULE_NAME}_OUT ${CONFORMANCE_PREFIX}${MODULE_NAME_LOWER}${CONFORMANCE_SUFFIX})

add_executable(${${MODULE_NAME}_OUT} ${${MODULE_NAME}_SOURCES})

set_property(TARGET ${${MODULE_NAME}_OUT} PROPERTY FOLDER "CONFORMANCE${CONFORMANCE_SUFFIX}")

set(HARNESS_LIB harness)

TARGET_LINK_LIBRARIES(${${MODULE_NAME}_OUT} ${HARNESS_LIB} ${CLConform_LIBRARIES})
message(${${MODULE_NAME}_OUT} " " ${HARNESS_LIB} " " ${CLConform_LIBRARIES})