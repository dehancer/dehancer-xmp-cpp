include(CMakePackageConfigHelpers)

FILE(GLOB PUBLIC_INCLUDE_FILES
        ../include/dehancer/*.h
        )

file (GLOB COMMON_SRC ${COMMON_SRC}
        ../src/*.cpp
        ../src/*.c
        )

message("PROJECT_NAME ${PROJECT_NAME} <- ${COMMON_SRC} ")

add_library(${PROJECT_NAME} ${COMMON_SRC})


if (COMMON_DEPENDENCIES)
    add_dependencies(${PROJECT_LIB} ${COMMON_DEPENDENCIES})
endif()

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}")
set(namespace "${PROJECT_NAME}::")

write_basic_package_version_file(
        "${version_config}" COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "cmake/Config.cmake.in"
        "${project_config}"
        INSTALL_DESTINATION "${config_install_dir}"
)

install(TARGETS ${PROJECT_LIB} DESTINATION lib)
install(FILES   ${PUBLIC_INCLUDE_FILES} DESTINATION include/dehancer)
install(FILES   ${project_config} ${version_config} DESTINATION lib/cmake/${PROJECT_LIB})
install(FILES   cmake/${PROJECT_LIB}.cmake DESTINATION lib/cmake/${PROJECT_LIB})
