cmake_minimum_required(VERSION 3.12)

#
# Project
#
set(PROJECT_LIB dehancer_xmp_cpp)
project(${PROJECT_LIB} VERSION 0.8.4)

#
# Options
#
option(BUILD_TESTING "Enable creation of tests." OFF)
option(DEHANCER_TARGET_ARCH "")
option(VCPKG_TARGET_TRIPLET "")
option(DEHANCER_DEBUG OFF)
option(USE_UNICODE OFF)

option(DEHANCER_COMMON_DIR  "")
if (NOT DEHANCER_COMMON_DIR)
    set(DEHANCER_COMMON_DIR ${CMAKE_BINARY_DIR}/tmp)
endif ()

option(EXTERNAL_INSTALL_LOCATION  "")
if (NOT EXTERNAL_INSTALL_LOCATION)
    set (EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
endif ()

if (DEHANCER_TARGET_ARCH)
    #
    # x86_64-apple-macos10.13
    # or
    # arm64-apple-macos11
    #
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target ${DEHANCER_TARGET_ARCH}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target ${DEHANCER_TARGET_ARCH}")
endif ()

#
# Global setttings
#
set(VERSION_FILE "version.txt")
set(CMAKE_CXX_STANDARD 17)

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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=1 -DZ7=1")
        add_definitions(/DEBUG /Z7)
    endif ()

    if (USE_UNICODE)
        add_definitions(/D_UNICODE /DUNICODE) # /utf-8)
    endif ()

else()
    set(CMAKE_INSTALL_LIBDIR  "lib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPRINT_DEBUG -DDEBUG_BUILD=1")

if (NOT APPLE)
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -static")
endif ()

# googletes, etc
set (EXTERNAL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external)
set (EXTERNAL_INCLUDE_DIR ${EXTERNAL_INSTALL_LOCATION}/include)
set (EXIVEDK_DB_DIR ${CMAKE_SOURCE_DIR}/external/exiv2)

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

if (${DEHANCER_TARGET_ARCH} MATCHES ".*arm64.*")
    MESSAGE("Configure: ARM4")

    find_package (ZLIB REQUIRED)

elseif (${DEHANCER_TARGET_ARCH} MATCHES ".*x86_64.*")
    MESSAGE("Configure: x86_64")

    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/x86_64")
    pkg_check_modules(ZLIB ZLIB)

    if(NOT ZLIB_FOUND)
        MESSAGE(FATAL "x86_64 zlib not found")
    endif()

    include_directories(
            ${ZLIB_INCLUDE_DIRS}
    )
    link_directories(
            ${ZLIB_LIBRARY_DIRS}
    )

else()
    find_package (ZLIB REQUIRED)
endif()


#
# Programs
#
find_program(CCACHE_FOUND ccache)
#find_program(MAKE_EXE NAMES gmake nmake make)

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

    if (NOT WIN32)
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
    endif ()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

    message(STATUS "Using gcc" )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Wno-ignored-qualifiers -Wvariadic-macros -pthread -static-libgcc -static-libstdc++")
    if (NOT WIN32)
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif ()
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
# Package manager
#
include(ExternalProject)

#
# External project dependencies
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

#
# Google Test
#
if (BUILD_TESTING)
    find_package(GTest)
    if (NOT GTEST_FOUND)

        string(STRIP "${CMAKE_C_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" C_FLAGS)
        string(STRIP "${CMAKE_CXX_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0" CXX_FLAGS)

        ExternalProject_Add(googletest
                GIT_REPOSITORY https://github.com/google/googletest
                GIT_PROGRESS YES
                GIT_SHALLOW YES
                GIT_TAG release-1.11.0
                CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                -DCMAKE_C_FLAGS=${C_FLAGS}
                -DCMAKE_CXX_FLAGS=${CXX_FLAGS}
                PREFIX ${DEHANCER_COMMON_DIR}
                )
        set(COMMON_DEPENDENCIES ${COMMON_DEPENDENCIES};googletest)
    endif ()

endif ()


set(EXIV2_FLAGS)

if (WIN32)
    set(EXIV2_FLAGS "-DXMP_WinBuild=1 -D_ITERATOR_DEBUG_LEVEL=0  ${CXX_FLAGS}")
else()
    #set(EXIV2_FLAGS "-fvisibility=hidden ${CXX_FLAGS}")
    set(EXIV2_FLAGS "-Wno-deprecated-copy -fvisibility=default ${CXX_FLAGS}")
endif ()


string(STRIP "${EXIV2_FLAGS}" EXIV2_FLAGS)

pkg_search_module(exiv2_lib exiv2)
if (NOT exiv2_lib_FOUND)
    ExternalProject_Add(exiv2_lib
            SOURCE_DIR ${EXIVEDK_DB_DIR}
            CMAKE_ARGS
            -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_FLAGS=${EXIV2_FLAGS}
            -DCMAKE_C_FLAGS=${C_FLAGS}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
            -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
            -DDEHANCER_TARGET_ARCH=${DEHANCER_TARGET_ARCH}
            -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DEXIV2_BUILD_SAMPLES=OFF
            -DEXIV2_BUILD_EXIV2_COMMAND=OFF
            -DEXIV2_ENABLE_XMP=ON
            -DBUILD_WITH_CCACHE=OFF
            -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF
            -DDEHANCER_TARGET_ARCH=${DEHANCER_TARGET_ARCH}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
            #BUILD_ALWAYS ON
            #LOG_DOWNLOAD ON
            #LOG_CONFIGURE ON
            #LOG_BUILD ON
            #LOG_INSTALL ON
            #LOG_OUTPUT_ON_FAILURE ON
            PREFIX ${DEHANCER_COMMON_DIR}
            )
    set(COMMON_DEPENDENCIES ${COMMON_DEPENDENCIES};exiv2_lib)
else()
    message(STATUS "exiv2_lib: ${exiv2_lib_LIBRARIES}, ${exiv2_lib_LIBRARY_DIRS} ${exiv2_lib_INCLUDE_DIRS}")
    include_directories(
            ${exiv2_lib_INCLUDE_DIRS}
    )
    link_directories(
            ${exiv2_lib_LIBRARY_DIRS}
    )
    set(COMMON_LIBRARIES ${exiv2_lib_LIBRARIES};${COMMON_LIBRARIES})

endif ()

pkg_search_module(dehancer_common_cpp dehancer-common-cpp)
if (NOT dehancer_common_cpp_FOUND)
    ExternalProject_Add(dcmcpp
            GIT_REPOSITORY https://github.com/dehancer/dehancer-common-cpp
            GIT_PROGRESS YES
            GIT_SHALLOW YES
            CMAKE_ARGS
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_C_FLAGS=${C_FLAGS}
            -DCMAKE_CXX_FLAGS=${CXX_FLAGS}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
            -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DDEHANCER_TARGET_ARCH=${DEHANCER_TARGET_ARCH}
            -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
            -DBUILD_TESTING=OFF
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DDEHANCER_COMMON_DIR=${DEHANCER_COMMON_DIR}
            -DEXTERNAL_INSTALL_LOCATION=${EXTERNAL_INSTALL_LOCATION}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
            PREFIX ${DEHANCER_COMMON_DIR}
            DEPENDS exiv2_lib
            )
    set(COMMON_DEPENDENCIES ${COMMON_DEPENDENCIES};dcmcpp)
    set(COMMON_LIBRARIES dehancer_common_cpp;ed25519cpp;base64cpp;exiv2;exiv2-xmp;${COMMON_LIBRARIES})
else()
    set(COMMON_LIBRARIES ${dehancer_common_cpp_LINK_LIBRARIES};exiv2;exiv2-xmp;${COMMON_LIBRARIES})
    include_directories(
            ${dehancer_common_cpp_INCLUDE_DIRS}
    )
endif ()

if (WIN32)
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
        ${EXTERNAL_INSTALL_LOCATION}/include
        ${CMAKE_SOURCE_DIR}/include
)

link_directories(
        ${EXTERNAL_INSTALL_LOCATION}/lib
        ${EXTERNAL_INSTALL_LOCATION}/lib64
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

configure_file(${CMAKE_SOURCE_DIR}/dehancer-xmp-cpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )
