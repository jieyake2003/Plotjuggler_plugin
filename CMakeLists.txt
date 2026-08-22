cmake_minimum_required(VERSION 3.5)
project(cell_voltage_bar_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ============================================================
# IMPORTANT: Set this to YOUR PlotJuggler 3.1.1 source directory
# ============================================================
set(PLOTJUGGLER_SOURCE_DIR "" CACHE PATH "Path to PlotJuggler 3.1.1 source tree")

if(NOT PLOTJUGGLER_SOURCE_DIR OR NOT EXISTS "${PLOTJUGGLER_SOURCE_DIR}/include/PlotJuggler")
    message(FATAL_ERROR
        "Please set PLOTJUGGLER_SOURCE_DIR to the PlotJuggler 3.1.1 source directory.\n"
        "  cmake -DPLOTJUGGLER_SOURCE_DIR=C:/path/to/PlotJuggler ..")
endif()

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

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Svg
    Qt5::Xml
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
message(STATUS "  Install directory:  ${PJ_PLUGIN_INSTALL_DIR}")
message(STATUS "================================")
