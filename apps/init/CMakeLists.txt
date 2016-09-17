
add_executable(init src/main.c src/serial.c src/serial.h)
target_link_libraries(init bedrock)
target_link_libraries(init resource)
install(TARGETS init DESTINATION images)
