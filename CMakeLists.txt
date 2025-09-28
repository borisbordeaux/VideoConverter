cmake_minimum_required(VERSION 3.16)

project(VideoConverter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "$ENV{QT_PATH}")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        commandinfo.cpp
        commandinfo.h
)

qt_add_executable(VideoConverter
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(VideoConverter PRIVATE Qt6::Widgets)

#include(GNUInstallDirs)

qt_finalize_executable(VideoConverter)
