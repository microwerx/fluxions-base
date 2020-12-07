cmake_minimum_required(VERSION 3.16)
project(fluxions-base)

set(CMAKE_C_STANDARD            99)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    include_directories(/usr/local/include)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME}
    STATIC
	src/fluxions_base.cpp
	src/fluxions_file_path_info.cpp
	src/fluxions_fileio_iostream.cpp
	src/fluxions_gl1gl2_tools.cpp
	src/fluxions_image_loader.cpp
	src/fluxions_opengl.cpp
	src/fluxions_simple_geometry_mesh.cpp
    src/fluxions_simple_map_library.cpp
    src/fluxions_simple_material_library.cpp
	src/fluxions_simple_renderer.cpp
	src/fluxions_xml.cpp
    )

if(${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    message("Fluxions Base using precompiled headers")
    target_precompile_headers(${PROJECT_NAME} PRIVATE src/fluxions_base_pch.cpp)
endif()

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
