cmake_minimum_required(VERSION 3.15)
project(whatsmy VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    main.cpp
    src/whatsmy.cpp
    src/backend/plugin_loader.cpp
    src/helpers/output.cpp
    src/helpers/error.cpp
)

# Executable
add_executable(whatsmy ${SOURCES})

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(whatsmy ${CMAKE_DL_LIBS})
elseif(APPLE)
    # macOS-specific libraries
    target_link_libraries(whatsmy ${CMAKE_DL_LIBS})
elseif(WIN32)
    # Windows-specific libraries (will add when needed)
endif()

# Installation
install(TARGETS whatsmy DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/whatsmy DESTINATION include)

# Options
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_PLUGINS "Enable plugin system" ON)

# Print configuration
message(STATUS "whatsmy version: ${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enable plugins: ${ENABLE_PLUGINS}")

