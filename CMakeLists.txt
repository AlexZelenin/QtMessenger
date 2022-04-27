cmake_minimum_required(VERSION 3.18)


set(PROJECT "QtMessenger")
project(${PROJECT})

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party)

include(${THIRD_PARTY_DIR}/cmake/Protobuf.cmake)

add_subdirectory(MsgServer)
add_subdirectory(MsgClient)
add_subdirectory(Test)
