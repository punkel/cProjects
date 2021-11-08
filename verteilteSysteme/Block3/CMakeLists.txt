cmake_minimum_required(VERSION 3.5)
project(Block3 C)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCE_FILES *.c *.h)
add_executable(client client.c message.c reader.c llist.c)
add_executable(server server.c message.c storageHandler.c)
