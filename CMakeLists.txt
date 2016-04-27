cmake_minimum_required(VERSION 3.3)
project(sel4_misc C ASM)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
# set(CMAKE_VERBOSE_MAKEFILE on)

add_definitions("-std=gnu11 -m32 -nostdinc -Wall")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -m32")

add_subdirectory("libs")
add_subdirectory("apps")

install(FILES ${CMAKE_SOURCE_DIR}/stage/x86/pc99/kernel.elf DESTINATION images)
