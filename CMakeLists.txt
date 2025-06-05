cmake_minimum_required(VERSION 3.10)

project(Guayadeque)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Release type for builds where CMAKE_BUILD_TYPE is unset.
# This is usually a good default as this implictly enables
#
#   CFLAGS = -O3 -DNDEBUG
#
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

message(STATUS "Building through C++ ${CMAKE_CXX_STANDARD} ...")
message(STATUS "Using install prefix ${CMAKE_INSTALL_PREFIX} ...")

find_package(PkgConfig REQUIRED)

set(SEARCH_LIBS /usr/lib)

# [sl 2020-05-23] according to https://cmake.org/cmake/help/v3.0/module/FindwxWidgets.html
set(wxWidgets_CONFIG_OPTIONS --toolkit=gtk3)

find_package(wxWidgets COMPONENTS base core adv net html xml aui OPTIONAL_COMPONENTS qa)

if (wxWidgets_FOUND)
    if (wxWidgets_LIBRARIES)
        if (wxWidgets_LIBRARIES MATCHES "wx_.*_qa-")   # wxWidgets_qa_FOUND does not work!
            add_definitions(-DUSE_WXQA=1)
        else()
            message(STATUS "wxWidgets QA component not found!")
        endif()
    else()
        message(FATAL_ERROR "wxWidgets libraries not found!")
    endif()
else()
    message(FATAL_ERROR "wxWidgets not found!")
endif()

find_package(ICU 60.0 COMPONENTS uc i18n REQUIRED)

pkg_check_modules(GSTREAMER gstreamer-1.0>=1.20)
if (NOT GSTREAMER_FOUND)
    check_required_components()
    pkg_check_modules(GSTREAMER gstreamer-1.0>=1.0)
    if (NOT GSTREAMER_FOUND)
        message(FATAL_ERROR "gstreamer 1.0 not found!")
    else()
        add_definitions(-DGSTREAMER_VERSION=1)
    endif()
else()
    add_definitions(-DGSTREAMER_VERSION=120)
endif()

pkg_check_modules(GSTREAMER REQUIRED gstreamer-pbutils-1.0)
pkg_check_modules(GSTREAMER_TAG REQUIRED gstreamer-tag-1.0)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(LIBDBUS REQUIRED dbus-1)
pkg_check_modules(LIBGDKPIXBUF20 REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(LIBGIO REQUIRED gio-2.0)

pkg_check_modules(LIBTAG taglib>=1.6.4)
if (NOT LIBTAG_FOUND)
    pkg_check_modules(LIBTAG taglib>=1.6.1)
    if (NOT LIBTAG_FOUND)
        pkg_check_modules(LIBTAG taglib>=1.6)
        if (NOT LIBTAG_FOUND)
            message(FATAL_ERROR "taglib 1.6/2.0 not found!")
        endif()
    else()
        add_definitions(-DTAGLIB_WITH_MP4_COVERS=1)
    endif()
else()
    if (LIBTAG_VERSION LESS 3.0)
        add_definitions(-DTAGLIB_WITH_MP4_COVERS=1)
        add_definitions(-DTAGLIB_WITH_APE_SUPPORT=1)
    else()
        message(FATAL_ERROR "taglib version must be less than 3.0!")
    endif()
endif()

pkg_check_modules(LIBWXSQLITE3 wxsqlite3)
if (NOT LIBWXSQLITE3_FOUND)
	pkg_check_modules(LIBWXSQLITE33 wxsqlite3-3.2)
	if (NOT LIBWXSQLITE33_FOUND)
		pkg_check_modules(LIBWXSQLITE33 wxsqlite3-3.0)
		if (NOT LIBWXSQLITE33_FOUND)
		    message(FATAL_ERROR "wxSqlite3 not found")
		endif()
	endif()
endif()

pkg_check_modules(LIBJSONCPP jsoncpp)
if (NOT LIBJSONCPP_FOUND)
  	message(FALTAL_ERROR "libjsoncpp not found")
endif()

option(ENABLE_IPOD "Enable iPod support by including libgpod support" ON)
if (ENABLE_IPOD)
    pkg_check_modules(LIBGPOD libgpod-1.0)
    if (NOT LIBGPOD_FOUND)
        message(" libgpod-dev not found! No iPod support!")
    else()
        add_definitions(-DWITH_LIBGPOD_SUPPORT=1)
    endif()
else()
    message(" *** iPod support disabled!")
endif()

# Include wxWidgets macros
include(${wxWidgets_USE_FILE})

if (NOT _GUREVISION_)
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE _GUREVISION_
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# Make Version.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

#add_definitions(${wxWidgets_DEFINITIONS})
add_compile_options(-Wall)
add_definitions(-DDATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}/guayadeque\" -DDATAROOTDIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}\")

# We define the include paths here, our minimal source dir is one,
# and also the include dirs defined by wxWidgets
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_TAG_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
    ${LIBTAG_INCLUDE_DIRS}
    ${LIBDBUS_INCLUDE_DIRS}
    ${LIBGIO_INCLUDE_DIRS}
    ${LIBGPOD_INCLUDE_DIRS}
    ${LIBWXSQLITE3_INCLUDE_DIRS}
    ${LIBWXSQLITE33_INCLUDE_DIRS}
    ${LIBJSONCPP_INCLUDE_DIRS}
    ${LIBICU_INCLUDE_DIRS}
)

if (PROFILE)
    add_definitions(-pg)
    set(CMAKE_EXE_LINKER_FLAGS "-pg -g")
    message("Adding profiling code.")
endif()

add_subdirectory(src)
add_subdirectory(po)

# default configuration and desktop icon
# freedesktop.org-specific files
install(FILES defconfig/guayadeque.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps/)
install(FILES defconfig/org.guayadeque.guayadeque.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES defconfig/org.guayadeque.guayadeque.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)

# program-specific files
install(FILES defconfig/guayadeque.default.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque)
install(FILES defconfig/equalizers.default.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque)
install(FILES defconfig/lyrics_sources.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/guayadeque)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

add_custom_target(debug
    ${CMAKE_COMMAND} -DMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})

add_custom_target(release
    ${CMAKE_COMMAND} -DMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})
