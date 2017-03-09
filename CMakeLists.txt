cmake_minimum_required(VERSION 3.3)
project(sel4_misc C ASM)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
# set(CMAKE_VERBOSE_MAKEFILE on)

execute_process(COMMAND ${CMAKE_C_COMPILER} -print-search-dirs
        COMMAND sed -n -e "s/install: \\\(.*\\\)/\\1/p"
        COMMAND tr -s "\n" "/"
        OUTPUT_VARIABLE CC_BUILTIN_ROOT)
set(CC_BUILTIN_INC ${CC_BUILTIN_ROOT}include)
add_definitions("-std=gnu11 -m32 -nostdinc -I ${CC_BUILTIN_INC} -ffreestanding -Wall -Wextra -O2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -m32")

add_subdirectory("libs")
add_subdirectory("apps")

install(FILES ${CMAKE_SOURCE_DIR}/stage/x86/pc99/kernel.elf DESTINATION images)
