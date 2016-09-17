
add_library(elfloader STATIC include src/elfparser.c include/elfloader.h src/elfloader.c)
target_include_directories(elfloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(elfloader bedrock)
target_link_libraries(elfloader resource)
