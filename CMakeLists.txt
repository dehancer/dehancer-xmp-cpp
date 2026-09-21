cmake_minimum_required(VERSION 4.3)
project(dehancer_xmp_cpp VERSION 0.16.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
option(CREATE_PKG_CONFIG "Generate and install pkg-config metadata" OFF)
option(BUILD_TESTING "Build tests" OFF)
option(DEHANCER_DEBUG "Enable release diagnostics" OFF)
option(PRINT_DEBUG "Enable logging" OFF)
option(USE_UNICODE "Use Unicode Windows APIs" OFF)

if(NOT TARGET dehancer_common_cpp::dehancer_common_cpp)
    find_package(dehancer_common_cpp CONFIG REQUIRED)
endif()

if(NOT TARGET Exiv2::exiv2lib)
    find_package(exiv2 CONFIG REQUIRED)
endif()

add_library(dehancer_xmp_cpp
    src/MLutXmp.cpp
    src/CameraLutXmp.cpp
)
add_library(dehancer_xmp_cpp::dehancer_xmp_cpp ALIAS dehancer_xmp_cpp)
target_compile_features(dehancer_xmp_cpp PUBLIC cxx_std_17)
set_target_properties(dehancer_xmp_cpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(dehancer_xmp_cpp PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_definitions(dehancer_xmp_cpp PRIVATE
    $<$<BOOL:${PRINT_DEBUG}>:PRINT_DEBUG=1>
    $<$<BOOL:${DEHANCER_DEBUG}>:DEBUG=1;DEBUG_BUILD=1>)
if(MSVC AND DEHANCER_DEBUG)
    target_compile_options(dehancer_xmp_cpp PRIVATE /Od /Ob0 /Z7 /UNDEBUG)
endif()
if(WIN32)
    target_compile_definitions(dehancer_xmp_cpp PUBLIC WIN32=1
        $<$<BOOL:${USE_UNICODE}>:UNICODE;_UNICODE>)
endif()
target_link_libraries(dehancer_xmp_cpp PUBLIC dehancer_common_cpp::dehancer_common_cpp Exiv2::exiv2lib)
option(DEHANCER_BLOWFISH_CACHE_DISABLED "Disable Blowfish cache" OFF)
target_compile_definitions(dehancer_xmp_cpp PRIVATE
    $<$<BOOL:${DEHANCER_BLOWFISH_CACHE_DISABLED}>:DEHANCER_BLOWFISH_CACHE_DISABLED=1>
    $<$<BOOL:${IOS_SYSTEM}>:IOS_SYSTEM=1>)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/dehancer_xmp_cpp")
configure_package_config_file(cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/dehancer_xmp_cppConfig.cmake"
    INSTALL_DESTINATION "${config_install_dir}")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/dehancer_xmp_cppConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)
install(TARGETS dehancer_xmp_cpp EXPORT dehancer_xmp_cppTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(EXPORT dehancer_xmp_cppTargets NAMESPACE dehancer_xmp_cpp:: DESTINATION "${config_install_dir}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/dehancer_xmp_cppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/dehancer_xmp_cppConfigVersion.cmake" DESTINATION "${config_install_dir}")

if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CREATE_PKG_CONFIG)
    if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
        set(prefix "${CMAKE_INSTALL_PREFIX}")
        set(libdir "${CMAKE_INSTALL_LIBDIR}")
    else()
        file(RELATIVE_PATH pc_prefix "/${CMAKE_INSTALL_LIBDIR}/pkgconfig" "/")
        set(prefix "\${pcfiledir}/${pc_prefix}")
        set(libdir "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
        set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()
    configure_file(dehancer-xmp-cpp.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dehancer-xmp-cpp.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
