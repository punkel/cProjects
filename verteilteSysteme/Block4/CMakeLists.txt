cmake_minimum_required(VERSION 3.5)
project(Block3 C)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCE_FILES *.c *.h)
add_executable(peer server.c message.c storageHandler.c llist.c com.c)
