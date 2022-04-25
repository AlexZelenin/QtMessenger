cmake_minimum_required(VERSION 3.21)


set(PROJECT "QtMessenger")
project(${PROJECT})


add_subdirectory(MsgServer)
add_subdirectory(MsgClient)
add_subdirectory(Test)
