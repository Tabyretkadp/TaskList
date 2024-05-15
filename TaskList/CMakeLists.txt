cmake_minimum_required(VERSION 3.10)

project(TaskList LANGUAGES CXX)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(TaskList
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(TaskList PRIVATE Qt5::Widgets)

