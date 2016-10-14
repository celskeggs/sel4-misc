
add_executable(sandbox src/main.c)
embed_binary(registrar image_registrar sandbox)
target_link_libraries(sandbox libbedrock)
target_link_libraries(sandbox libinit)
target_link_libraries(sandbox libipc)
