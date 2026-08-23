cmake_minimum_required(VERSION 3.5)
project(cell_voltage_bar_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ============================================================
# IMPORTANT: Set this to YOUR PlotJuggler 3.1.1 source/build directory
# ============================================================
set(PLOTJUGGLER_SOURCE_DIR "" CACHE PATH "Path to PlotJuggler 3.1.1 source tree")
set(PLOTJUGGLER_BUILD_DIR "" CACHE PATH "Path to PlotJuggler 3.1.1 build directory (where libplotjuggler_base.lib lives)")

if(NOT PLOTJUGGLER_SOURCE_DIR OR NOT EXISTS "${PLOTJUGGLER_SOURCE_DIR}/include/PlotJuggler")
    message(FATAL_ERROR
        "Please set PLOTJUGGLER_SOURCE_DIR to the PlotJuggler 3.1.1 source directory.\n"
        "  cmake -DPLOTJUGGLER_SOURCE_DIR=C:/path/to/PlotJuggler ..")
endif()

# If PLOTJUGGLER_BUILD_DIR not set, assume it's the build subdir inside source dir
if(NOT PLOTJUGGLER_BUILD_DIR)
    set(PLOTJUGGLER_BUILD_DIR "${PLOTJUGGLER_SOURCE_DIR}/build")
endif()

# ---- Find PlotJuggler base library ----
find_library(PJ_BASE_LIB
    NAMES plotjuggler_base
    PATHS
        ${PLOTJUGGLER_BUILD_DIR}
        ${PLOTJUGGLER_BUILD_DIR}/Release
        ${PLOTJUGGLER_BUILD_DIR}/Debug
        ${PLOTJUGGLER_SOURCE_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT PJ_BASE_LIB)
    message(FATAL_ERROR
        "Cannot find libplotjuggler_base. Did you build PlotJuggler 3.1.1 first?\n"
        "  Expected location: ${PLOTJUGGLER_BUILD_DIR}\n"
        "  Set -DPLOTJUGGLER_BUILD_DIR=... to the correct build path.")
endif()

message(STATUS "Found PlotJuggler base library: ${PJ_BASE_LIB}")

# ---- Include Paths ----
set(PJ_INCLUDE_DIRS
    ${PLOTJUGGLER_SOURCE_DIR}/include          # PlotJuggler/plotdata.h etc.
    ${PLOTJUGGLER_SOURCE_DIR}/src              # plotwidget.h, plot_docker.h, customtracker.h etc.
    ${PLOTJUGGLER_SOURCE_DIR}/3rdparty/qwt/src # Qwt headers
    ${PLOTJUGGLER_SOURCE_DIR}/3rdparty/Qt-Advanced-Docking/src
)

# ---- Qt Dependencies ----
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg Xml)
# ---- Generate MOC for PlotJuggler base classes ----
qt5_wrap_cpp(PJ_MOC_SRC
    ${PLOTJUGGLER_SOURCE_DIR}/include/PlotJuggler/statepublisher_base.h
)
Skip to content
Plotjuggler_plugin
Repository navigation
Code
Issues
Pull requests
Build PlotJuggler Plugin
Build PlotJuggler Plugin #8
Annotations
1 error and 1 warning
build
failed 1 minute ago in 1m 6s
4s
4s
6s
5s
37s
8s
Run mkdir build
CMake Deprecation Warning at CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.


-- The CXX compiler identification is MSVC 19.44.35228.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
CMake Error at CMakeLists.txt:37 (message):
  Cannot find libplotjuggler_base.  Did you build PlotJuggler 3.1.1 first?

    Expected location: D:/a/Plotjuggler_plugin/Plotjuggler_plugin/pj_src/build
    Set -DPLOTJUGGLER_BUILD_DIR=... to the correct build path.


-- Configuring incomplete, errors occurred!
NMAKE : fatal error U1052: file 'Makefile' not found
Stop.
Error: Process completed with exit code 2.
0s
0s
1s
0s

# ---- Build the Plugin ----
add_library(${PROJECT_NAME} SHARED
    cell_voltage_bar_plugin.h
    cell_voltage_bar_plugin.cpp
    ${PJ_MOC_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PJ_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# ---- Link libraries (NOW INCLUDES PlotJuggler base!) ----
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Svg
    Qt5::Xml
    ${PJ_BASE_LIB}
)

# ---- Copy JSON metadata to build output ----
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cell_voltage_bar_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/cell_voltage_bar_plugin.json
    COPYONLY
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/cell_voltage_bar_plugin.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/cell_voltage_bar_plugin.json
    COMMENT "Copying plugin metadata to output directory"
)

# ---- Installation ----
set(PJ_PLUGIN_INSTALL_DIR "$ENV{LOCALAPPDATA}/PlotJuggler/plugins" CACHE PATH
    "PlotJuggler plugin installation directory")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${PJ_PLUGIN_INSTALL_DIR}
    RUNTIME DESTINATION ${PJ_PLUGIN_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cell_voltage_bar_plugin.json
    DESTINATION ${PJ_PLUGIN_INSTALL_DIR}
)

message(STATUS "=== Cell Voltage Bar Plugin ===")
message(STATUS "  PlotJuggler source: ${PLOTJUGGLER_SOURCE_DIR}")
message(STATUS "  PlotJuggler library: ${PJ_BASE_LIB}")
message(STATUS "  Install directory:  ${PJ_PLUGIN_INSTALL_DIR}")
message(STATUS "================================")
