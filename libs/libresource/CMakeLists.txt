
# "exported" - used by other CMake scripts
set(LIB_RESOURCE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(libresource STATIC ${LIB_RESOURCE_HEADERS}
        src/cslot.c
        src/mem_arena.c
        src/mem_fx.c
        src/mem_fxalloc.c
        src/mem_fxcache.c
        src/mem_fxlarge.c
        src/mem_fxseq.c
        src/mem_page.c
        src/mem_vspace.c
        src/object.c
        src/untyped.c)
target_include_directories(libresource PUBLIC ${LIB_RESOURCE_HEADERS})
target_link_libraries(libresource libbedrock)
