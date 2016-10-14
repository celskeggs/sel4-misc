macro(embed_binary target_name symbol_name include_into)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_bin.c
            COMMAND python2 ${CMAKE_SOURCE_DIR}/tools/incbin.py $<TARGET_FILE:${target_name}> ${symbol_name}
            ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_bin.c
            DEPENDS ${target_name} ${CMAKE_SOURCE_DIR}/tools/incbin.py)
    target_sources(${include_into} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_bin.c)
endmacro()

add_subdirectory("init")
add_subdirectory("sandbox")
add_subdirectory("helloworld")
add_subdirectory("registrar")
