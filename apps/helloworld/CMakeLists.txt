
add_executable(helloworld src/main.c)
target_link_libraries(helloworld libbedrock)
target_link_libraries(helloworld libinit)

install(TARGETS helloworld DESTINATION images)
