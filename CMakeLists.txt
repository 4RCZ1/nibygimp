cmake_minimum_required(VERSION 3.30)
project(nibygimp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
    if(NOT Qt6_FOUND)
        message(STATUS "QT NOT FOUND. Using predefined path.")
        set(CMAKE_PREFIX_PATH "C:\\Qt\\6.8.2\\mingw_64" CACHE PATH "QT path")
    else ()
        message(STATUS "Found Qt6 in: ${Qt6_DIR}")
        set(CMAKE_PREFIX_PATH ${Qt6_DIR} CACHE PATH "QT path")
    endif()
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(nibygimp main.cpp
        src/image/PPM.cpp
        src/image/PPM.h
        src/image/Image.cpp
        src/image/Image.h
        src/files/FileManager.cpp
        src/files/FileManager.h
        src/tools/Greyscale.cpp
        src/tools/Greyscale.h
        src/tools/Histogram.cpp
        src/tools/Histogram.h
        src/tools/HistogramDisplay.cpp
        src/tools/HistogramDisplay.h
        src/tools/Blur.cpp
        src/tools/Blur.h)
target_link_libraries(nibygimp
  Qt::Core
  Qt::Gui
  Qt::Widgets
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()
    foreach(QT_LIB Core Gui Widgets )
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(QT_LIB)
endif()
