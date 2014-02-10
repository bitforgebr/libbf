find_package(GTest REQUIRED)
find_library(LIBPTHREAD NAMES pthread)
find_library(LIBGTEST NAMES gtest)
find_library(LIBGTEST_MAIN NAMES gtest_main)
find_package(LibBF HINTS $ENV{MISC_CMAKE_MODULES} REQUIRED)

find_library(LIBEV NAMES ev)
find_path(LIBEV_INCLUDE_DIR "ev++.h" HINTS /usr/libev /usr/local/libev /usr/local/include /usr/include /usr/include/libev)

include_directories(${LIBEV_INCLUDE_DIR} ../)

add_executable( bfsocket-tests udptests.cpp tcptests.cpp libevtests.cpp )

target_link_libraries( bfsocket-tests ${LIBEV} ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${PROTOBUF_LIBRARIES} pthread ${LIBGTEST}  ${LIBGTEST_MAIN} crypto ev bf)

add_test(
    NAME bfsocket-tests
    COMMAND bfsocket-tests
)
