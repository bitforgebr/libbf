



if(EXISTS "/etc/slackware-version")
    #add_definitions(--sysroot=/root/prod-sysroot -isysroot / --no-sysroot-suffix)
    file (READ "${CMAKE_HOME_DIRECTORY}/VERSION" VERSION)
    math(EXPR VERSION "${VERSION}+1")
    file(WRITE "${CMAKE_HOME_DIRECTORY}/VERSION" ${VERSION})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(STRIP_ARGS -R .comment)
else()
    set(STRIP_ARGS -R .comment -R .GCC.command.line --strip-unneeded -s -g)
endif()

add_definitions(-Wno-write-strings -Wall -Wextra -ftrack-macro-expansion=2)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-O1 -Wl,--as-needed -Wl,--hash-style=gnu")

if ( CPP11 )
    message(STATUS "Using C++ 11")
    add_definitions(${CPP11_DEFS})
else()
    message(FATAL_ERROR "Need a C++ 11 compiler!")
endif()

include_directories( . ${PROTOBUF_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} )

add_library( bfsocket STATIC bfsocket.cpp utils.h)

target_link_libraries( bfsocket ${LibBitForge_LIBRARIES} )

get_property(output TARGET bfsocket PROPERTY LOCATION)

add_subdirectory(test)
