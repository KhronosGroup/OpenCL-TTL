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
 
 cmake_minimum_required (VERSION 3.3)

project(TemplateTilingLibrary)

include(GNUInstallDirs)

set(TTL_COMMON_FILES
    TTL.h
    TTL_trace_macros.h
    TTL_import_export.h
    TTL_macros.h
    TTL_types.h
    TTL_pipeline_schemes.h
    TTL_debug.h
    tensors/TTL_ext_tensors.h
    tensors/TTL_int_tensors.h
    tensors/TTL_tensors_common.h
    TTL_core.h
    TTL_tensors.h
    TTL_tiles.h
    pipelines/TTL_double_scheme.h
    pipelines/TTL_schemes_common.h
    pipelines/TTL_simplex_scheme.h
    pipelines/TTL_double_scheme_template.h
    pipelines/TTL_duplex_scheme.h
)

set(TTL_HEADER_C_FILES
    ${TTL_COMMON_FILES}
    c/TTL_import_export.h
    c/TTL_types.h
)

set(TTL_HEADER_OPENCL_FILES
    ${TTL_COMMON_FILES}
    opencl/TTL_import_export.h
    opencl/TTL_types.h
    opencl/TTL_async_work_group_copy_3D3D.h
)

if(DEFINED TTL_INSTALL_DIR)
    if(NOT IS_ABSOLUTE ${TTL_INSTALL_DIR})
        set(TTL_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/${TTL_INSTALL_DIR}")
    endif()
else()
    set(TTL_INSTALL_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
endif()

set(TTL_INSTALL_DIR "${TTL_INSTALL_DIR}" CACHE PATH "The install location for all installation types.")

set(TTL_C_INSTALL_PATH 
    ${TTL_INSTALL_DIR}/TTL)
set(TTL_OPENCL_INSTALL_PATH 
    ${TTL_INSTALL_DIR}/TTL)

macro(InstallTTL type install_directory ttl_header_files)
    install(CODE "MESSAGE(\"Removing old TTL ${type} Library from ${install_directory}.\")")
    file (REMOVE_RECURSE ${install_directory})

    install(CODE "MESSAGE(\"Installing TTL ${type} Library in ${install_directory}.\")")

    foreach (file ${ttl_header_files} )
        get_filename_component(dir ${file} DIRECTORY)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/${file} DESTINATION ${install_directory}/${dir})
    endforeach()
endmacro()

InstallTTL("C" ${TTL_C_INSTALL_PATH} "${TTL_HEADER_C_FILES}")
InstallTTL("OpenCL" ${TTL_OPENCL_INSTALL_PATH} "${TTL_HEADER_OPENCL_FILES}")
