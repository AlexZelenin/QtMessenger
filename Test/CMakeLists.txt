cmake_minimum_required(VERSION 3.5)

project(TestMessenger LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Test Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../MsgServer)
include_directories(../MsgClient)

add_executable(TestMessenger tst_testmessenger.cpp)
add_test(NAME TestMessenger COMMAND TestMessenger)

target_link_libraries(TestMessenger PRIVATE
                            Qt${QT_VERSION_MAJOR}::Test
                            Qt${QT_VERSION_MAJOR}::Widgets
                            Qt${QT_VERSION_MAJOR}::Network
                     )

