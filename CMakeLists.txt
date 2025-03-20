cmake_minimum_required(VERSION 3.5)

project(VitalSyncPro VERSION 1.0 LANGUAGES CXX)

# Set output directories to be within the build directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Basic Qt components only
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Define source groups
set(INCLUDE_FILES
    include/config_manager.h
    include/i_data_manager.h
    include/i_data_provider.h
    include/i_parameter_model.h
    include/i_parameter_view.h
    include/i_waveform_model.h
    include/i_waveform_view.h
    include/vital_sync_types.h
)

set(CORE_FILES
    src/core/config_manager.cpp
    src/core/data_manager.cpp
    src/core/data_manager.h
    src/core/parameter_model.cpp
    src/core/parameter_model.h
    src/core/waveform_model.cpp
    src/core/waveform_model.h
)

set(PROVIDERS_FILES
    src/providers/demo_data_provider.cpp
    src/providers/demo_data_provider.h
)

set(UI_FILES
    src/ui/main_window.cpp
    src/ui/main_window.h
    src/ui/provider_config_dialog.cpp
    src/ui/provider_config_dialog.h
    src/ui/settings_dialog.cpp
    src/ui/settings_dialog.h
    src/ui/parameters/parameter_view.cpp
    src/ui/parameters/parameter_view.h
    src/ui/waveforms/waveform_view.cpp
    src/ui/waveforms/waveform_view.h
)

set(UTILS_FILES
    # Utility files
)

# Main application entry point
set(MAIN_FILE
    src/main.cpp
)

# Combine all source files - explicitly excluding legacy files
set(PROJECT_SOURCES
    ${INCLUDE_FILES}
    ${CORE_FILES}
    ${PROVIDERS_FILES}
    ${UI_FILES}
    ${UTILS_FILES}
    ${MAIN_FILE}
)

# Create executable 
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VitalSyncPro
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(VitalSyncPro SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(VitalSyncPro
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Set include directories
target_include_directories(VitalSyncPro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries - just what we need
target_link_libraries(VitalSyncPro PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Set target properties
set_target_properties(VitalSyncPro PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.vitalsynctech.vitalsyncpro
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VitalSyncPro)
endif()

# Important build information message
message(STATUS "=======================================================")
message(STATUS "BUILDING INSTRUCTIONS")
message(STATUS "To build this project, use the following commands:")
message(STATUS "  1. mkdir -p build")
message(STATUS "  2. cd build")
message(STATUS "  3. cmake ..")
message(STATUS "  4. make")
message(STATUS "")
message(STATUS "")
message(STATUS "=======================================================")
