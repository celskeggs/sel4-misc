
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helloworld_bin.c
        COMMAND python2 ${CMAKE_CURRENT_SOURCE_DIR}/src/incbin.py $<TARGET_FILE:helloworld> image_helloworld
        ${CMAKE_CURRENT_BINARY_DIR}/helloworld_bin.c
        DEPENDS helloworld ${CMAKE_CURRENT_SOURCE_DIR}/src/incbin.py)

add_executable(init src/main.c src/serial.c src/serial.h ${CMAKE_CURRENT_BINARY_DIR}/helloworld_bin.c)
target_link_libraries(init libbedrock)
target_link_libraries(init libresource)
target_link_libraries(init libelfloader)
install(TARGETS init DESTINATION images)
