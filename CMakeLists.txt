cmake_minimum_required(VERSION 3.14)

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)  # project() command manages VERSION variables
endif ()
if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)  # option() honors normal variables
endif ()
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)  # ExternalProject download/update timestamps
endif ()

project(xcompute VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code)
set(XCOMPUTE_DIR ${CODE_DIR}/${PROJECT_NAME})
set(VENDOR_DIR ${CODE_DIR}/Vendor)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

include(FetchContent)

# Set FetchContent properties to suppress warnings
set(FETCHCONTENT_QUIET ON)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)  # For dependencies that don't set this policy

# ============================================================================
# GLFW - Window and input handling
# ============================================================================
message(STATUS "Fetching GLFW...")

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
)

# Configure GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# ============================================================================
# OpenGL - Graphics API
# ============================================================================
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
    ${VENDOR_DIR}/glad.c
    ${XCOMPUTE_DIR}/ComputeRunner.hpp
    ${XCOMPUTE_DIR}/ComputeRunner.cpp
    ${XCOMPUTE_DIR}/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CODE_DIR} ${VENDOR_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
)

if (WIN32)
    # Windows-specific libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        opengl32
        gdi32
        user32
        kernel32
    )
elseif (APPLE)
    # macOS-specific frameworks
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif (UNIX)
    # Linux-specific libraries
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )

    # X11 libraries for Linux
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
endif ()

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif ()

if (APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GL_SILENCE_DEPRECATION
    )
endif ()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Shaders)

# Copy example shaders if they exist
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/")
    file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*")
    foreach (SHADER_FILE ${SHADER_FILES})
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
        configure_file(${SHADER_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders/${SHADER_NAME} COPYONLY)
    endforeach ()
endif ()

message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "OpenGL library: ${OPENGL_LIBRARY}")
message(STATUS "===================================")
message(STATUS "")