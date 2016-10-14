
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sandbox_bin.c
        COMMAND python2 ${CMAKE_CURRENT_SOURCE_DIR}/src/incbin.py $<TARGET_FILE:sandbox> image_sandbox
        ${CMAKE_CURRENT_BINARY_DIR}/sandbox_bin.c
        DEPENDS sandbox ${CMAKE_CURRENT_SOURCE_DIR}/src/incbin.py)

add_executable(init
        src/main.c src/serial.c src/serial.h src/untyped.c src/untyped.h src/small.c src/small.h src/objectsrv.c
        ${CMAKE_CURRENT_BINARY_DIR}/sandbox_bin.c)
target_link_libraries(init libbedrock)
target_link_libraries(init libresource)
target_link_libraries(init libelfloader)
target_link_libraries(init libipc)
install(TARGETS init DESTINATION images)
