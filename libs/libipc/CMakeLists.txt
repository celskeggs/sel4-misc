
add_library(libipc STATIC include src/core.c)
target_include_directories(libipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(libipc libbedrock)
target_link_libraries(libipc libresource)
