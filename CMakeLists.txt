cmake_minimum_required(VERSION 3.22)

option(DEHANCER_DEBUG OFF)

set(GIT_TAG_dehancer_common_cpp "develop")

if(NOT DEHANCER_DEBUG)
    set(GIT_TAG_dehancer_common_cpp "0.16.0")
endif ()

#
# Project
#
set(PROJECT_LIB dehancer_xmp_cpp)
project(${PROJECT_LIB} VERSION 0.13.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED OFF)

message(STATUS "CXX_FILESYSTEM_HAVE_FS:	${CXX_FILESYSTEM_HAVE_FS}")
message(STATUS "CXX_FILESYSTEM_IS_EXPERIMENTAL:	${CXX_FILESYSTEM_IS_EXPERIMENTAL}")

#
# Options
#
option(GTest_ROOT "Google Test SDK" "")
option(BUILD_TESTING "Enable creation of tests." OFF)
option(VCPKG_TARGET_TRIPLET "")
option(USE_UNICODE OFF)
option(DEHANCER_CONTROLLED_SINGLETON OFF)

option(DEHANCER_BLOWFISH_CACHE_DISABLED OFF)
if(DEHANCER_BLOWFISH_CACHE_DISABLED)
    add_definitions(-DDEHANCER_BLOWFISH_CACHE_DISABLED=1)
endif()

if(IOS_SYSTEM)
    add_definitions(-DIOS_SYSTEM=1)
endif()

option(EXTERNAL_INSTALL_LOCATION  "")
if (NOT EXTERNAL_INSTALL_LOCATION)
    set (EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
endif ()

#
# Global settings
#
set(VERSION_FILE "version.txt")

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc -Wno-deprecated-declarations")
    add_definitions(/MT /DWIN32=1 /D_ITERATOR_DEBUG_LEVEL=0)

    if (DEHANCER_DEBUG)
        string(REPLACE "/O2" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/Ob2" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/DNDEBUG" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    endif ()

    if (USE_UNICODE)
        add_definitions(/D_UNICODE /DUNICODE) # /utf-8)
    endif ()

else()
    set(CMAKE_INSTALL_LIBDIR  "lib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPRINT_DEBUG -DDEBUG_BUILD=1")

#
# Generators
#
file (WRITE "${VERSION_FILE}" "${PROJECT_NAME} ${PROJECT_VERSION}")
configure_file(include/dehancer/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)

#
# Packages
#
find_package (PkgConfig QUIET)
if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found!" )
    message(FATAL_ERROR "sudo apt-get install pkg-config, or brew install pkg-config")
else()
    set (PKG_CONFIG_USE_CMAKE_PREFIX_PATH 1)
endif()

find_package (EXPAT REQUIRED)
find_package (Iconv REQUIRED)
find_package (ZLIB REQUIRED)

#
# Programs
#
find_program(CCACHE_FOUND ccache)

#
# CCache settings
#
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


#
# Compiler settings
#

message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})

if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(LINUX ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DX86_LINUX")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")

    message(STATUS "Using Clang" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wc++17-extensions")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

    message(STATUS "Using gcc" )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Wno-ignored-qualifiers -Wvariadic-macros -pthread -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pedantic")
    set(CMAKE_LINK_DEPENDS_NO_SHARED TRUE)

    if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DX86_LINUX")
    endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")

    message( FATAL_ERROR "The project does not support Intel compiler" )

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message( FATAL_ERROR "The project does not support MSVC compiler" )

endif()

#
# Project dependencies
#
set(COMMON_DEPENDENCIES)
set(COMMON_LIBRARIES
        ${CMAKE_THREAD_LIBS_INIT};
        ${MKL_LIBRARIES};
        ${EXPAT_LIBRARIES};
        ${Iconv_LIBRARIES};
        ${ZLIB_LIBRARIES};
        )

if (MINGW)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES};ws2_32;kernel32;psapi)
endif ()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc -Wno-deprecated-declarations")
    add_definitions(/MT /DWIN32=1 /D_ITERATOR_DEBUG_LEVEL=0)
    string(TOLOWER  ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    if (${BUILD_TYPE} STREQUAL "debug")
        add_definitions(/MTd)
    endif()
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif ()

string(STRIP "${CMAKE_C_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" C_FLAGS)
string(STRIP "${CMAKE_CXX_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" CXX_FLAGS)

set(FETCHCONTENT_QUIET FALSE)

#
# Google Test
#
if (BUILD_TESTING)

    find_package(GTest)

    if (NOT GTest_FOUND)

        string(STRIP "${CMAKE_C_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" C_FLAGS)
        string(STRIP "${CMAKE_CXX_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" CXX_FLAGS)

        set(CMAKE_OSX_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}" CACHE STRING "" FORCE)

        include(FetchContent)

        FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG      v1.14.0
                GIT_PROGRESS TRUE
                GIT_SHALLOW TRUE
                SOURCE_DIR "${CMAKE_BINARY_DIR}/googletest-src"
        )

        FetchContent_GetProperties(googletest)

        if(NOT googletest_POPULATED)
            FetchContent_Populate(googletest)
            add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

    endif ()

endif ()

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build static libraries")
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE INTERNAL "Build with CMAKE_POSITION_INDEPENDENT_CODE lib")

#
# Exiv2
#
set(EXIV2_ENABLE_BROTLI OFF CACHE INTERNAL "Build without BROTLI lib")
set(EXIV2_ENABLE_INIH OFF CACHE INTERNAL "Build without INIH lib")
set(EXIV2_BUILD_SAMPLES OFF CACHE INTERNAL "Build with CMAKE_POSITION_INDEPENDENT_CODE lib")
set(EXIV2_BUILD_EXIV2_COMMAND OFF CACHE INTERNAL "Build without EXIV2_COMMAND ")
set(EXIV2_ENABLE_XMP ON CACHE INTERNAL "Build without XMP lib")
set(EXIV2_ENABLE_DYNAMIC_RUNTIME OFF CACHE INTERNAL "Build without DYNAMIC_RUNTIME")
set(EXIV2_ENABLE_FILESYSTEM_ACCESS OFF CACHE INTERNAL "Build without EXIV2_ENABLE_FILESYSTEM_ACCESS")
if (WIN32)
    set(CMAKE_CXX_FLAGS "-DXMP_WinBuild=1 -D_ITERATOR_DEBUG_LEVEL=0 ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "Build with XMP_WinBuild")
else()
    set(CMAKE_CXX_FLAGS "-Wno-deprecated-copy ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "Build with -Wno-deprecated-copy")
endif ()

set(DEBUG_GLOBAL ${DEBUG})
set(DEBUG OFF CACHE INTERNAL "DEBUG disabled for exiv2" )
FetchContent_Declare(
        exiv2
        GIT_REPOSITORY  https://github.com/Exiv2/exiv2.git
        GIT_TAG     v0.28.2
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        SOURCE_DIR "${CMAKE_BINARY_DIR}/exiv2-src"
        BINARY_DIR "${CMAKE_BINARY_DIR}/exiv2-build"
        INSTALL_COMMAND ${CMAKE_COMMAND} -E cmake --build . --target=install
)

FetchContent_MakeAvailable(exiv2)

include_directories(${exiv2_SOURCE_DIR}/include)
include_directories(${exiv2_SOURCE_DIR}/include/exiv2)
include_directories(${CMAKE_BINARY_DIR})

set(DEBUG ${DEBUG_GLOBAL} CACHE INTERNAL "DEBUG restored")


#
# dehancer-common-cpp
#
FetchContent_Declare(
        dehancer_common_cpp
        GIT_REPOSITORY  https://github.com/dehancer/dehancer-common-cpp
        GIT_TAG     ${GIT_TAG_dehancer_common_cpp}
        SOURCE_DIR "${CMAKE_BINARY_DIR}/dehancer_common_cpp-src"
        BINARY_DIR "${CMAKE_BINARY_DIR}/dehancer_common_cpp-build"
)

FetchContent_MakeAvailable(dehancer_common_cpp)

include_directories(${dehancer_common_cpp_SOURCE_DIR}/include)
include_directories(${dehancer_common_cpp_SOURCE_DIR}/external)
include_directories(${base64cpp_SOURCE_DIR}/include)

set(COMMON_LIBRARIES ${COMMON_LIBRARIES};dehancer_common_cpp;ed25519cpp;base64cpp;exiv2lib)
set(COMMON_DEPENDENCIES ${COMMON_DEPENDENCIES};dehancer_common_cpp;exiv2lib)

if (WIN32)
    add_definitions(/DEBUG /Z7)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES};psapi)
endif ()

if (LINUX)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES};pthread)
endif ()

message("COMMON_LIBRARIES: ${COMMON_LIBRARIES}")

#
# Details
#
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}/lib
)

add_subdirectory(lib)

if(BUILD_TESTING)
    add_subdirectory(tests)
    enable_testing ()
endif()


# Install package
#
if (NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR  lib)
endif ()
if (NOT CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR  include)
endif ()

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dehancer-xmp-cpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )
