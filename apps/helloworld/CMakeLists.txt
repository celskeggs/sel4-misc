
add_executable(helloworld src/main.c)
target_link_libraries(helloworld bedrock)
target_link_libraries(helloworld resource)

install(TARGETS helloworld DESTINATION images)
