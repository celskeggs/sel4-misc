
add_library(libelfloader STATIC include src/elfparser.c src/elfloader.c src/elfexec.c)
target_include_directories(libelfloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(libelfloader libbedrock)
target_link_libraries(libelfloader libresource)
