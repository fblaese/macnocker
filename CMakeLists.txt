cmake_minimum_required(VERSION 3.0)

project(macNock C)

option(MACNOCK_DEBUG "Enable debug messages" OFF)

add_definitions(-Wall -Wextra -pedantic)

if(MACNOCK_DEBUG)
    add_definitions(-DDEBUG)
endif(MACNOCK_DEBUG)

set(MACNOCK_SRC
    main.c
    macnockserver.c
    macnockclient.c
    tc.c
    macstorage.c
    )

add_executable(macnock ${MACNOCK_SRC})
target_link_libraries(macnock pthread)

install(TARGETS macnock RUNTIME DESTINATION sbin)
