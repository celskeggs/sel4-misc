
add_library(libinit STATIC include src/init.c src/objectcli.c src/execcli.c)
target_include_directories(libinit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(libinit libbedrock)
target_link_libraries(libinit libresource)
target_link_libraries(libinit libelfloader)  # TODO: make this not be a dependency
target_link_libraries(libinit libipc)
