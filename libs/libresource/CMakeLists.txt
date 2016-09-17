
# "exported" - used by other CMake scripts
set(LIB_RESOURCE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(resource STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cslot_ao.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_arena.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_fx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_fxalloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_fxcache.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_fxseq.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_page.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_vspace.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/object.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/untyped.c
        ${LIB_RESOURCE_HEADERS})
target_include_directories(resource PUBLIC ${LIB_RESOURCE_HEADERS})
target_link_libraries(resource bedrock)
