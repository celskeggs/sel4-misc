
get_property(SEL4_BEDROCK_INCLUDE_DIRECTORIES TARGET sel4 PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
string(REPLACE ";" ";-I" SEL4_BEDROCK_INCLUDE ";${SEL4_BEDROCK_INCLUDE_DIRECTORIES}")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/arch-x86/_start.s
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/src/arch-x86/
        COMMAND "${CMAKE_C_COMPILER}" -E ${CMAKE_CURRENT_SOURCE_DIR}/src/arch-x86/_start.S -nostdinc -o ${CMAKE_CURRENT_BINARY_DIR}/src/arch-x86/_start.s ${SEL4_BEDROCK_INCLUDE}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/src/arch-x86/_start.S)

# "exported" - used by other CMake scripts
set(LIB_BEDROCK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(bedrock STATIC ${LIB_BEDROCK_HEADERS}
        src/errx.c src/assert.c src/buffer.c src/debug.c ${CMAKE_CURRENT_BINARY_DIR}/src/arch-x86/_start.s)
target_include_directories(bedrock PUBLIC ${LIB_BEDROCK_HEADERS})
target_link_libraries(bedrock sel4)
target_link_libraries(bedrock -lgcc)
