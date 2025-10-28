cmake_minimum_required(VERSION 3.15)
project(whatsmy VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags (common warnings)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-g -O0)
        # Optional: Address sanitizer for debugging
        # add_compile_options(-fsanitize=address)
        # add_link_options(-fsanitize=address)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
        add_compile_definitions(NDEBUG)
    else()
        add_compile_options(-O3)
        add_compile_definitions(NDEBUG)
        # Strip symbols on Unix-like systems
        if(UNIX)
            add_link_options(-s)
        endif()
    endif()
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

# Platform-specific source files
if(UNIX AND NOT APPLE)
    # Linux
    list(APPEND SOURCES src/backend/platform/linux.cpp)
elseif(APPLE)
    # macOS
    list(APPEND SOURCES src/backend/platform/macos.cpp)
elseif(WIN32)
    # Windows
    list(APPEND SOURCES src/backend/platform/windows.cpp)
endif()

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

# Install headers (for plugin development)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/whatsmy DESTINATION include)

# Create plugin directory during installation
if(UNIX)
    # Linux/macOS: Install to /usr/lib/whatsmy/plugins or /usr/local/lib/whatsmy/plugins
    install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/whatsmy/plugins
            DIRECTORY_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
elseif(WIN32)
    # Windows: Install to Program Files/whatsmy/plugins
    install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/whatsmy/plugins)
endif()

# Options
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_PLUGINS "Enable plugin system" ON)

# Print configuration
message(STATUS "whatsmy version: ${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enable plugins: ${ENABLE_PLUGINS}")

