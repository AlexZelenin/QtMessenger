cmake_minimum_required(VERSION 3.18)


project(MsgServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB PROTOBUF_DEFINITION_FILES "proto/*.proto")

set(PROTOBUF_INPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/proto")
set(PROTOBUF_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")

set (PROTOBUF_ROOT ${THIRD_PARTY_DIR}/protobuf)

set(Protobuf_USE_STATIC_LIBS ON)

include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-config.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-module.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-options.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-targets.cmake)

set(Protobuf_INCLUDE_DIR ${PROTOBUF_ROOT}/include)

find_package(Protobuf REQUIRED)
if ( Protobuf_FOUND )
    message( STATUS "Protobuf version : ${Protobuf_VERSION}" )
    message( STATUS "Protobuf include path : ${Protobuf_INCLUDE_DIRS}" )
    message( STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}" )
    message( STATUS "Protobuf compiler libraries : ${Protobuf_PROTOC_LIBRARIES}")
    message( STATUS "Protobuf lite libraries : ${Protobuf_LITE_LIBRARIES}")
else()
    message( WARNING "Protobuf package not found -> specify search path via PROTOBUF_ROOT variable")
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})

file(GLOB PROTOBUF_MODELS_INCLUDES "*.pb.cc" "*pb.h")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOBUF_DEFINITION_FILES})

qt_add_resources(QT_RESOURCES resources.qrc)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        connectedtabbar.h
        connectedtabbar.cpp
        connectedtabbar.ui
        server.h
        server.cpp
        sthread.h
        sthread.cpp
        ${PROTOBUF_DEFINITION_FILES}
        ${PROTO_SRCS}
        ${PROTO_HDRS}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MsgServer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${QT_RESOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MsgServer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MsgServer SHARED
            ${PROJECT_SOURCES}
            ${QT_RESOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MsgServer
            ${PROJECT_SOURCES}
            ${QT_RESOURCES}
        )
    endif()
endif()

target_link_libraries(MsgServer PRIVATE
                        Qt${QT_VERSION_MAJOR}::Widgets
                        Qt${QT_VERSION_MAJOR}::Network
                        ${PROTOBUF_LIBRARY}
                     )

set_target_properties(MsgServer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MsgServer)
endif()
