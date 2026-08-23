cmake_minimum_required(VERSION 3.10)
project(cell_voltage_bar_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ============================================================
# PlotJuggler 3.1.1 路径配置
# ============================================================
set(PLOTJUGGLER_SOURCE_DIR "" CACHE PATH "Path to PlotJuggler 3.1.1 source tree")
set(PLOTJUGGLER_BUILD_DIR "" CACHE PATH "Path to PlotJuggler 3.1.1 build directory")

if(NOT PLOTJUGGLER_SOURCE_DIR OR NOT EXISTS "${PLOTJUGGLER_SOURCE_DIR}/include/PlotJuggler")
    message(FATAL_ERROR
        "PLOTJUGGLER_SOURCE_DIR is invalid or not set.\n"
        "Pass -DPLOTJUGGLER_SOURCE_DIR=C:/path/to/PlotJuggler ..")
endif()

if(NOT PLOTJUGGLER_BUILD_DIR)
    set(PLOTJUGGLER_BUILD_DIR "${PLOTJUGGLER_SOURCE_DIR}/build")
endif()

# ---- 查找 PlotJuggler 核心库 ----
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
        "Cannot find libplotjuggler_base.lib.\n"
        "Did you build PlotJuggler 3.1.1 first?\n"
        "Expected in: ${PLOTJUGGLER_BUILD_DIR}\n"
        "Set -DPLOTJUGGLER_BUILD_DIR=... to the correct path.")
endif()

message(STATUS "Found PlotJuggler base library: ${PJ_BASE_LIB}")

# ---- 头文件路径 ----
set(PJ_INCLUDE_DIRS
    ${PLOTJUGGLER_SOURCE_DIR}/include
    ${PLOTJUGGLER_SOURCE_DIR}/src
    ${PLOTJUGGLER_SOURCE_DIR}/3rdparty/qwt/src
    ${PLOTJUGGLER_SOURCE_DIR}/3rdparty/Qt-Advanced-Docking/src
)

# ---- Qt5 ----
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg Xml)

# ---- MOC ----
qt5_wrap_cpp(PJ_MOC_SRC
    ${PLOTJUGGLER_SOURCE_DIR}/include/PlotJuggler/statepublisher_base.h
)

# ---- 编译插件 ----
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
    ${PJ_BASE_LIB}
)

# ---- 拷贝 JSON 到输出目录 ----
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cell_voltage_bar_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/cell_voltage_bar_plugin.json
    COPYONLY
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/cell_voltage_bar_plugin.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/cell_voltage_bar_plugin.json
)

# ---- 安装 ----
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
message(STATUS "  PJ Source: ${PLOTJUGGLER_SOURCE_DIR}")
message(STATUS "  PJ Library: ${PJ_BASE_LIB}")
message(STATUS "================================")
