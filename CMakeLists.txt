cmake_minimum_required(VERSION 3.0)

project(macNock C CXX)

option(MACNOCK_DEBUG "Enable debug messages" OFF)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wpedantic -Wextra -std=c++11)
endif()

if(MACNOCK_DEBUG)
    add_definitions(-DDEBUG)
endif(MACNOCK_DEBUG)

set(MACNOCK_SRC
    main.cpp
    macnockserver.cpp
    macnockclient.cpp
    nockpackage.cpp
    mac.cpp
    tc.cpp
    log.cpp
    )

add_executable(macnock ${MACNOCK_SRC})
target_link_libraries(macnock pthread)

install(TARGETS macnock RUNTIME DESTINATION sbin)
