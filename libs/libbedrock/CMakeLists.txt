
# "exported" - used by other CMake scripts
set(LIB_RESOURCE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(bedrock STATIC ${LIB_RESOURCE_HEADERS}
        src/errno.c src/assert.c src/buffer.c src/debug.c)
target_include_directories(bedrock PUBLIC ${LIB_RESOURCE_HEADERS})
target_link_libraries(bedrock sel4)
target_link_libraries(bedrock -lgcc)