
add_executable(registrar src/main.c)
target_link_libraries(registrar libbedrock)
target_link_libraries(registrar libinit)
target_link_libraries(registrar libipc)
