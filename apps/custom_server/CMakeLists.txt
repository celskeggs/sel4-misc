
get_property(CUSTOM_SERVER_INCLUDE_DIRECTORIES TARGET sel4 PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
string(REPLACE ";" ";-I" CUSTOM_SERVER_INCLUDE ";${CUSTOM_SERVER_INCLUDE_DIRECTORIES}")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/
        COMMAND "${CMAKE_C_COMPILER}" -E ${CMAKE_CURRENT_SOURCE_DIR}/crt/arch-x86/crt0.S -nostdinc -o ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s ${CUSTOM_SERVER_INCLUDE}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/crt/arch-x86/crt0.S)

add_executable(custom_server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c ${CMAKE_CURRENT_SOURCE_DIR}/src/pc99/serial.c
        ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s
        src/alloc/cslot_ao.c src/alloc/cslot_ao.h src/alloc/mem_ao.c src/alloc/mem_ao.h src/basic.h
        src/alloc/untyped.c src/alloc/untyped.h src/alloc/object.c src/alloc/object.h src/alloc/mem_page.c src/alloc/mem_page.h src/errno.h src/errno.c src/destructor.c src/destructor.h)
target_link_libraries(custom_server sel4)
target_link_libraries(custom_server -lgcc)

install(TARGETS custom_server DESTINATION images)
