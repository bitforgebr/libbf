project(libbf)

cmake_minimum_required(VERSION 2.8)

############### Copmpilation #####################################

add_definitions(-Wall -Wextra -fPIC)

find_package(CheckForCPP11 HINTS $ENV{MISC_CMAKE_MODULES})

if ( CPP11 )
    message(STATUS "Using C++ 11")
    add_definitions(${CPP11_DEFS})
else()
    message(FATAL_ERROR "Need a C++ 11 compiler!")
endif()

include_directories(.)

add_library(bf bf/bf.cpp bf/log.cpp)

############### Intallation and Packaging #####################################

install(TARGETS bf DESTINATION lib)
install(FILES bf/bf.h bf/ncstring.h bf/log.h DESTINATION include/bf)

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
