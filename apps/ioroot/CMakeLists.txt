
add_executable(ioroot src/main.c)
target_link_libraries(ioroot libbedrock)
target_link_libraries(ioroot libinit)
target_link_libraries(ioroot libipc)
