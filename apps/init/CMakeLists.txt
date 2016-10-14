
add_executable(init
        src/main.c src/serial.c src/serial.h src/untyped.c src/untyped.h src/small.c src/small.h src/objectsrv.c)
embed_binary(sandbox image_sandbox init)
target_link_libraries(init libbedrock)
target_link_libraries(init libresource)
target_link_libraries(init libelfloader)
target_link_libraries(init libipc)
install(TARGETS init DESTINATION images)
