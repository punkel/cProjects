cmake_minimum_required(VERSION 3.13)
project(Block02 C)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCE_FILES *.c *.h)
set(SOURCE_FILES  server.c llist.c llist.h quoatReader.c quoatReader.h)
add_executable(server ${SOURCE_FILES})
add_executable(client client.c)
