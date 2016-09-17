
get_property(SERVER_INCLUDE_DIRECTORIES TARGET sel4 PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
string(REPLACE ";" ";-I" SERVER_INCLUDE ";${SERVER_INCLUDE_DIRECTORIES}")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/
        COMMAND "${CMAKE_C_COMPILER}" -E ${CMAKE_CURRENT_SOURCE_DIR}/crt/arch-x86/crt0.S -nostdinc -o ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s ${SERVER_INCLUDE}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/crt/arch-x86/crt0.S)

add_executable(init
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
        ${CMAKE_CURRENT_BINARY_DIR}/crt/arch-x86/crt0.s
        src/serial.c src/serial.h)
target_link_libraries(init bedrock)
target_link_libraries(init resource)

install(TARGETS init DESTINATION images)
