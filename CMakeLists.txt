cmake_minimum_required(VERSION 3.5)

project(machine_sim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR on)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Mqtt PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Mqtt PrintSupport)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        doubleclickslider.h
        axis.h
        axis.cpp
        machine.h
        machine.cpp
        qcustomplot.h
        qcustomplot.cpp
        toml.hpp
)


qt_add_executable(machine_sim
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

if(WIN32)
    set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj")
endif()

target_link_libraries(machine_sim PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(machine_sim PRIVATE Qt${QT_VERSION_MAJOR}::Mqtt)
target_link_libraries(machine_sim PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)

set_target_properties(machine_sim PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS machine_sim
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(machine_sim)
endif()
