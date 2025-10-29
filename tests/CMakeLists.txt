# Tests CMakeLists.txt for whatsmycli
# Copyright (C) 2025 enXov
# Licensed under GPLv3

# Unit Tests
add_executable(unit_tests
    unit/test_command_parser.cpp
    unit/test_plugin_loader.cpp
    unit/test_helpers.cpp
    # Source files needed for testing (exclude main.cpp)
    ${CMAKE_SOURCE_DIR}/src/whatsmy.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/plugin_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/plugin_validator.cpp
    ${CMAKE_SOURCE_DIR}/src/helpers/output.cpp
    ${CMAKE_SOURCE_DIR}/src/helpers/error.cpp
)

# Platform-specific sources for unit tests
if(UNIX AND NOT APPLE)
    target_sources(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/linux.cpp)
elseif(APPLE)
    target_sources(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/macos.cpp)
elseif(WIN32)
    target_sources(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/windows.cpp)
endif()

# Link Google Test and required libraries
target_link_libraries(unit_tests 
    GTest::gtest_main
    GTest::gmock
)

# Platform-specific libraries for unit tests
if(UNIX)
    target_link_libraries(unit_tests ${CMAKE_DL_LIBS})
endif()

# Include directories for unit tests
target_include_directories(unit_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# Discover and register unit tests with CTest
gtest_discover_tests(unit_tests)

# Integration Tests
add_executable(integration_tests
    integration/test_e2e.cpp
    integration/test_plugin_execution.cpp
    # Source files needed for testing (exclude main.cpp)
    ${CMAKE_SOURCE_DIR}/src/whatsmy.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/plugin_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/plugin_validator.cpp
    ${CMAKE_SOURCE_DIR}/src/helpers/output.cpp
    ${CMAKE_SOURCE_DIR}/src/helpers/error.cpp
)

# Platform-specific sources for integration tests
if(UNIX AND NOT APPLE)
    target_sources(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/linux.cpp)
elseif(APPLE)
    target_sources(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/macos.cpp)
elseif(WIN32)
    target_sources(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/backend/platform/windows.cpp)
endif()

# Link Google Test and required libraries
target_link_libraries(integration_tests 
    GTest::gtest_main
    GTest::gmock
)

# Platform-specific libraries for integration tests
if(UNIX)
    target_link_libraries(integration_tests ${CMAKE_DL_LIBS})
endif()

# Include directories for integration tests
target_include_directories(integration_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# Discover and register integration tests with CTest
gtest_discover_tests(integration_tests)

# Copy test data/fixtures if needed
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

