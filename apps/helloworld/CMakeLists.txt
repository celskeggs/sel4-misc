
add_executable(helloworld src/main.c)
target_link_libraries(helloworld bedrock)
target_link_libraries(helloworld resource)
target_link_libraries(helloworld elfloader) # TODO: make this not be a dependency

install(TARGETS helloworld DESTINATION images)
