
add_executable(helloworld src/main.c)
target_link_libraries(helloworld libbedrock)
target_link_libraries(helloworld libresource)
target_link_libraries(helloworld libelfloader) # TODO: make this not be a dependency

install(TARGETS helloworld DESTINATION images)
