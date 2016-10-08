
add_executable(sandbox src/main.c)
target_link_libraries(sandbox libbedrock)
target_link_libraries(sandbox libinit)
target_link_libraries(sandbox libipc)
