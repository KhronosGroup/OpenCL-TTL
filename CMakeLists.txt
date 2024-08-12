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

if(DEFINED TTL_PRE_GENERATE)
set(TTL_COMMON_FILES
	TTL.h
    TTL_macros.h
)

set(TTL_HEADER_C_FILES
	TTL_c.h
	${TTL_COMMON_FILES}
)
set(TTL_HEADER_OPENCL_FILES
	TTL_opencl.h
	${TTL_COMMON_FILES}
)
set(TTL_SOURCE_DIRECTORY
    ${CMAKE_BINARY_DIR}
)

if(DEFINED TTL_COPY_3D)
    set(TTL_EXTRA_DEFINES "${TTL_EXTRA_DEFINES} -DTTL_COPY_3D")
    set(TTL_COPY_3D "${TTL_COPY_3D}" CACHE PATH "Defined if precompiled TTL should provide an implementation of async_work_group_copy_3D3D.")
endif()

if(DEFINED TTL_DEBUG)
    set(TTL_EXTRA_DEFINES "${TTL_EXTRA_DEFINES} -D__TTL_DEBUG=1")
    set(TTL_DEBUG "${TTL_DEBUG}" CACHE PATH "Defined if precompiled TTL should include debug information.")
endif()

set(TTL_PRE_GENERATE "${TTL_PRE_GENERATE}" CACHE STRING "The pre-generation option chosen.")

else()
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

set(TTL_SOURCE_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}
)

if(DEFINED TTL_COPY_3D)
    message(FATAL_ERROR "TTL_COPY_3D not a valid option for when TTL_PRE_GENERATE is not set")
endif()

if(DEFINED TTL_DEBUG)
    message(FATAL_ERROR "TTL_DEBUG not a valid option for when TTL_PRE_GENERATE is not set")
endif()

endif()

if(DEFINED TTL_INSTALL_PATH)
    if(NOT IS_ABSOLUTE ${TTL_INSTALL_PATH})
        set(TTL_INSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/${TTL_INSTALL_PATH}")
    endif()
else()
    set(TTL_INSTALL_PATH ${CMAKE_INSTALL_FULL_INCLUDEDIR})
endif()

set(TTL_INSTALL_PATH "${TTL_INSTALL_PATH}" CACHE PATH "The install location for all installation types.")

if(DEFINED TTL_C_INSTALL_PATH)
    if(NOT IS_ABSOLUTE ${TTL_C_INSTALL_PATH})
        set(TTL_C_INSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/${TTL_C_INSTALL_PATH}")
    endif()
else()
    set(TTL_C_INSTALL_PATH ${TTL_INSTALL_PATH})
endif()

if(DEFINED TTL_OPENCL_INSTALL_PATH)
    if(NOT IS_ABSOLUTE ${TTL_OPENCL_INSTALL_PATH})
        set(TTL_OPENCL_INSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/${TTL_OPENCL_INSTALL_PATH}")
    endif()
else()
    set(TTL_OPENCL_INSTALL_PATH ${TTL_INSTALL_PATH})
endif()

macro(InstallTTL type source_directory install_directory ttl_header_files)
    install(CODE "execute_process(COMMAND \"${CMAKE_CURRENT_SOURCE_DIR}/generation/generate_single_TTL.sh\" \"${CMAKE_BINARY_DIR}/TTL.h\" \"${type}\" \"${TTL_EXTRA_DEFINES}\")")
    install(CODE "MESSAGE(\"${CMAKE_CURRENT_SOURCE_DIR}/generation/generate_single_TTL.sh ${CMAKE_BINARY_DIR}/TTL.h ${type} ${TTL_EXTRA_DEFINES}\")")

    install(CODE "MESSAGE(\"Removing old TTL ${type} Library from ${install_directory}.\")")
    file (REMOVE_RECURSE ${install_directory})

    install(CODE "MESSAGE(\"Installing TTL ${type} Library in ${install_directory}.\")")

    foreach (file ${ttl_header_files} )
        get_filename_component(dir ${file} DIRECTORY)
        install(FILES ${source_directory}/${file} DESTINATION ${install_directory}/${dir})
    endforeach()
endmacro()

if(DEFINED TTL_PRE_GENERATE)
	add_custom_target(PrecompiledHeaders ALL
		COMMENT "Generating precompiled file before installation"
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/preprocess.sh -o TTL_opencl.h
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/preprocess.sh -o TTL_c.h -t c
		COMMAND cp ${CMAKE_SOURCE_DIR}/scripts/preprocess_ttl.h TTL.h
		COMMAND cp ${CMAKE_SOURCE_DIR}/TTL_macros.h TTL_macros.h
	)
endif()

InstallTTL("c" ${TTL_SOURCE_DIRECTORY} ${TTL_C_INSTALL_PATH} "${TTL_HEADER_C_FILES}")
InstallTTL("opencl" ${TTL_SOURCE_DIRECTORY} ${TTL_OPENCL_INSTALL_PATH} "${TTL_HEADER_OPENCL_FILES}")

if(DEFINED TTL_PRE_GENERATE)
    message("TTL will be precompiled before installation")
endif()

message("C files will be installed to ${TTL_C_INSTALL_PATH}")
message("OpenCL files will be installed to ${TTL_OPENCL_INSTALL_PATH}")

if(DEFINED TTL_COPY_3D)
    message("TTL will include an implementation of async_work_group_copy_3D3D")
endif()
