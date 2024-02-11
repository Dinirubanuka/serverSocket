cmake_minimum_required(VERSION 3.24)
project(Serversocket)

set(CMAKE_CXX_STANDARD 17)

add_executable(Serversocket SocketUtil.h SocketUtil.c main.c
        new)
