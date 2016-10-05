
add_library(libinit STATIC include src/init.c)
target_include_directories(libinit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(libinit libbedrock) # TODO: fix it so that users of libinit don't have to link to libbedrock first
target_link_libraries(libinit libresource)
target_link_libraries(libinit libelfloader)  # TODO: make this not be a dependency
