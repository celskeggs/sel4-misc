
add_library(liblua STATIC
        src/lapi.c
        src/lauxlib.c
        src/lbaselib.c
        src/lbitlib.c
        src/lcode.c
        src/lcorolib.c
        src/lctype.c
        src/ldblib.c
        src/ldebug.c
        src/ldo.c
        src/ldump.c
        src/lfunc.c
        src/lgc.c
        src/linit.c
        src/llex.c
        src/lmem.c
        src/lobject.c
        src/lopcodes.c
        src/loslib.c
        src/lparser.c
        src/lstate.c
        src/lstring.c
        src/lstrlib.c
        src/ltable.c
        src/ltablib.c
        src/ltm.c
        src/lua.c
        src/luac.c
        src/lundump.c
        src/lutf8lib.c
        src/lvm.c
        src/lzio.c
        src/llimits.h
        include)
target_include_directories(liblua PUBLIC include)
target_link_libraries(liblua libbedrock)
