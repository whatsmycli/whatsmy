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
        # Maximum speed optimization
        add_compile_options(/O2 /GL)
        add_compile_definitions(NDEBUG)
        # Link-time code generation
        add_link_options(/LTCG /OPT:REF /OPT:ICF)
    else()
        # Maximum optimization with size consideration
        add_compile_options(-O3 -flto)
        add_compile_definitions(NDEBUG)
        
        # Enable Link-Time Optimization (LTO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        
        # Strip symbols on Unix-like systems
        if(UNIX)
            add_link_options(-s)
        endif()
        
        # Static linking for better portability (Linux only)
        if(UNIX AND NOT APPLE)
            # Static link libgcc and libstdc++ for portability
            add_link_options(-static-libgcc -static-libstdc++)
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
    src/backend/plugin_validator.cpp
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
    # Linux/macOS: Install to lib/whatsmy/plugins (relative to install prefix)
    install(DIRECTORY DESTINATION lib/whatsmy/plugins
            DIRECTORY_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
elseif(WIN32)
    # Windows: Install to lib/whatsmy/plugins (relative to install prefix)
    install(DIRECTORY DESTINATION lib/whatsmy/plugins)
endif()

# Options
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_PLUGINS "Enable plugin system" ON)

# Print configuration
message(STATUS "whatsmy version: ${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enable plugins: ${ENABLE_PLUGINS}")
message(STATUS "Build testing: ${BUILD_TESTING}")

# Testing configuration
if(BUILD_TESTING)
    enable_testing()
    
    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    # Make Google Test available
    FetchContent_MakeAvailable(googletest)
    
    # Include Google Test's CMake utilities
    include(GoogleTest)
    
    # Add tests subdirectory
    add_subdirectory(tests)
endif()

# ============================================================================
# Packaging Configuration (CPack)
# ============================================================================

include(InstallRequiredSystemLibraries)

# Basic package information
set(CPACK_PACKAGE_NAME "whatsmy")
set(CPACK_PACKAGE_VENDOR "enXov")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast, minimal, and extensible cross-platform CLI tool for system information")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "https://github.com/whatsmycli/whatsmy")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Platform-specific packaging
if(UNIX AND NOT APPLE)
    # Linux: TGZ (tar.gz) archives
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SYSTEM_NAME "linux-x64")
    set(CPACK_PACKAGE_FILE_NAME "whatsmy-${PROJECT_VERSION}-linux-x64")
    
elseif(APPLE)
    # macOS: TGZ (tar.gz) archives
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SYSTEM_NAME "macos-universal")
    set(CPACK_PACKAGE_FILE_NAME "whatsmy-${PROJECT_VERSION}-macos-universal")
    
elseif(WIN32)
    # Windows: ZIP archives
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_SYSTEM_NAME "windows-x64")
    set(CPACK_PACKAGE_FILE_NAME "whatsmy-${PROJECT_VERSION}-windows-x64")
endif()

# Include CPack module
include(CPack)

