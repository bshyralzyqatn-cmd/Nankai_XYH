cmake_minimum_required(VERSION 3.16)

project(untitled VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 关键修复 1：加上 Multimedia 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

qt_standard_project_setup()

qt_add_executable(untitled
    main.cpp
    mainwindow.cpp
    mainwindow.h
    gameobjects.h
    res.qrc

)

# 关键修复 2：链接 Multimedia 库
target_link_libraries(untitled PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)