project(libbf)

cmake_minimum_required(VERSION 2.8)

############### Copmpilation #####################################

add_definitions(-Wall -Wextra -ftrack-macro-expansion=2 -fPIC)

set(CURSES_NEED_NCURSES True)
find_package(Curses)

if (CURSES_LIBRARIES)
    set(curses_files
        bf/ncurses/ncapplication.cpp
        bf/ncurses/ncform.cpp
        bf/ncurses/ncmenu.cpp
        bf/ncurses/ncmenu.cpp
        bf/ncurses/ncwidget.cpp
        bf/ncurses/ncwindow.cpp
        bf/ncurses/ncstatusbar.cpp
    )

    set(curses_install_headers
        bf/ncurses/ncapplication.h
        bf/ncurses/ncform.h
        bf/ncurses/ncmenu.h
        bf/ncurses/ncmenu.h
        bf/ncurses/ncwidget.h
        bf/ncurses/ncwindow.h
        bf/ncurses/ncstatusbar.h
    )

    include_directories(${CURSES_INCLUDE_PATH} ${CURSES_INCLUDE_PATH}/ncurses)
else()
    set(CURSES_LIBRARIES )
    message(STATUS "NCurses not found.")
endif()

find_package(CheckForCPP11 HINTS $ENV{MISC_CMAKE_MODULES})

if ( CPP11 )
    message(STATUS "Using C++ 11")
    add_definitions(${CPP11_DEFS})
else()
    message(FATAL_ERROR "Need a C++ 11 compiler!")
endif()


include_directories(.)

add_library(bf
    bf/bf.cpp
    bf/log.cpp
    ${CURSES_LIBRARIES}
    ${curses_files}

    bf/io/bfio.cpp
    bf/io/bfserial.cpp
    bf/io/net/bfsocket.cpp
)

############### Unit tests ####################################################

set(build_tests false CACHE BOOL "Set to TRUE to build unit tests")

if(build_tests)
    find_package(GTest REQUIRED)
    find_library(LIBGTEST NAMES gtest)
    find_library(LIBGTEST_MAIN NAMES gtest_main)
    find_package(Boost COMPONENTS date_time REQUIRED)

    enable_testing()


    add_executable(runUnitTests tests/int_hex_tests.cpp tests/circularbuffer_test.cpp tests/utils_tests.cpp tests/log_test.cpp)
    target_link_libraries(runUnitTests bf ${Boost_LIBRARIES} ${LIBGTEST_MAIN} ${LIBGTEST} pthread)

    add_test(
        NAME runUnitTests
        COMMAND runUnitTests
    )

    if (CURSES_LIBRARIES)
        add_executable(ncurses tests/ncurses.cpp)
        target_link_libraries(ncurses bf ${Boost_LIBRARIES}  pthread ${CURSES_LIBRARIES})
    endif()

    add_subdirectory(bf/io/net/test)

endif()

############### Intallation and Packaging #####################################

install(TARGETS bf DESTINATION lib)
install(FILES
    bf/bf.h
    bf/ncstring.h
    bf/log.h
    bf/buffers.h
    bf/inthex.h
    bf/service.h

    ${curses_install_headers}

    bf/io/bfio.h
    bf/io/bfserial.h
    
    bf/io/net/bfsocket.h
    
    DESTINATION include/bf)

set(CPACK_GENERATOR "RPM;TGZ;STGZ;TBZ2")

# Genral CPACK stuff
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "OPM Traceroute")
set(CPACK_PACKAGE_NAME                  "opm-traceroute")
set(CPACK_PACKAGE_VERSION               "0.1")
set(CPACK_PACKAGE_VENDOR                "orange")
set(CPACK_PACKAGE_DESCRIPTION_FILE      "${CMAKE_SOURCE_DIR}/README.md")

# CPACK RPM-specific stuff
set(CPACK_RPM_PACKAGE_RELEASE           1)
set(CPACK_RPM_PACKAGE_LICENSE           "orange-iptv-license")
set(CPACK_RPM_PACKAGE_GROUP             "orange")
set(CPACK_RPM_PACKAGE_REQUIRES          "libcurl, openssl, protobuf")

include(CPack)
