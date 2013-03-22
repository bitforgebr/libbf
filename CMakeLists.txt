project(libbf)

cmake_minimum_required(VERSION 2.8)

############### Copmpilation #####################################

add_definitions(-Wall -Wextra -ftrack-macro-expansion=2 -fPIC)

find_package(CheckForCPP11 HINTS $ENV{MISC_CMAKE_MODULES})

if ( CPP11 )
    message(STATUS "Using C++ 11")
    add_definitions(${CPP11_DEFS})
else()
    message(FATAL_ERROR "Need a C++ 11 compiler!")
endif()

include_directories(.)

add_library(bf bf/bf.cpp bf/log.cpp)

############### Unit tests ####################################################

set(build_tests false CACHE BOOL "Set to TRUE to build unit tests")

if(build_tests)
    find_package(GTest REQUIRED)
    find_library(LIBGTEST NAMES gtest)
    find_library(LIBGTEST_MAIN NAMES gtest_main)
    find_package(Boost COMPONENTS date_time REQUIRED)
    
    enable_testing()
    
    add_executable(runUnitTests tests/int_hex_tests.cpp tests/circularbuffer_test.cpp tests/log_test.cpp)
    
    target_link_libraries(runUnitTests bf ${Boost_LIBRARIES} ${LIBGTEST_MAIN} ${LIBGTEST})
    
    add_test(
        NAME runUnitTests
        COMMAND runUnitTests
    )
endif()

############### Intallation and Packaging #####################################

install(TARGETS bf DESTINATION lib)
install(FILES bf/bf.h bf/ncstring.h bf/log.h bf/buffers.h bf/inthex.h DESTINATION include/bf)

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
