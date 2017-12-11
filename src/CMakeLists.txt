cmake_minimum_required(VERSION 3.7)

project(purplepp)

add_library(purplepp

        # Source
        #test.c
        library.cpp
        core/connection.cpp
        core/account.cpp
        core/status.cpp
        core/saved_status.cpp
        core/conversation.cpp
        core/blist.cpp
        detail/util.cpp
        detail/internal.cpp

        # Headers for IDE
        "${PURPLEPP_INCLUDE_DIR}/purple++/purple++.h"
        #"${PURPLEPP_INCLUDE_DIR}/purple++/test.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/library.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/connection.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/account.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/status.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/saved_status.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/conversation.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/core/blist.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/detail/util.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/detail/internal.h"
        "${PURPLEPP_INCLUDE_DIR}/purple++/detail/lambda_visitor.h"
        )

find_path(PURPLE_INCLUDE_DIR NAMES purple.h PATH_SUFFIXES libpurple)

# TODO: maybe simplify (?)
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GLIB glib-2.0)
PKG_CHECK_MODULES(GTHREAD gthread-2.0)
PKG_CHECK_MODULES(GOBJECT gobject-2.0)
SET(GLIB_LIBS glib-2.0 gthread-2.0 gobject-2.0)
EXEC_PROGRAM(pkg-config ARGS --cflags ${GLIB_LIBS} OUTPUT_VARIABLE GLIB2_CFLAGS RETURE_VALUE PKG_CONFIG_RETURE_VALUE)
EXEC_PROGRAM(pkg-config ARGS --libs ${GLIB_LIBS} OUTPUT_VARIABLE GLIB2_LIBS RETURN_VALUE PKG_CONFIG_RETURE_VALUE)
MESSAGE(${GLIB2_CFLAGS})
MESSAGE(${GLIB2_LIBS})

find_package(Boost 1.62.0)
if(Boost_FOUND)
    target_include_directories(purplepp PRIVATE ${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found")
endif()

target_compile_options(purplepp PRIVATE
        -Wall
        )

# libpurple and dependencies
target_include_directories(purplepp PRIVATE
        ${GLIB2_CFLAGS} ${GLIB_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS}
        ${PURPLE_INCLUDE_DIR}
        )

# C++ libs
target_include_directories(purplepp PRIVATE
        ${PURPLEPP_INCLUDE_DIR}
        ${FMT_INCLUDE_DIRS}
        )
target_link_libraries(purplepp
        ${GLIB2_LIBS}
        purple
        fmt::fmt-header-only
        )