function(add_xmp_test target directory fixture)
    set(source_dir "${CMAKE_CURRENT_SOURCE_DIR}/${directory}")
    set(binary_dir "${CMAKE_CURRENT_BINARY_DIR}/${directory}")

    add_executable(${target} "${source_dir}/xmp_open.cpp")
    target_link_libraries(${target} PRIVATE dehancer_xmp_cpp GTest::gtest_main)

    configure_file("${source_dir}/${fixture}" "${binary_dir}/${fixture}" COPYONLY)
    if(EXISTS "${source_dir}/.env")
        configure_file("${source_dir}/.env" "${binary_dir}/.env" COPYONLY)
    elseif(NOT EXISTS "${binary_dir}/.env")
        file(WRITE "${binary_dir}/.env" "CMLUT_KEY=\n")
    endif()

    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES WORKING_DIRECTORY "${binary_dir}")
endfunction()

add_xmp_test(mlut-xmp-open-test mlut_xmp mlut.mlut)
add_xmp_test(clut-xmp-open-test cameralut_xmp clut.clut)
