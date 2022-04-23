install(DIRECTORY
        ${EXTERNAL_INSTALL_LOCATION}/include/exiv2 DESTINATION include
        FILES_MATCHING PATTERN *.h PATTERN *.hpp
        )

message("CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")

if (MSVC)
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/exiv2.lib DESTINATION lib)
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/exiv2-xmp.lib DESTINATION lib)
else()
    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message("Dehancer xmp ios version and exiv2 have to build separately...")
    else()
        install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/libexiv2.a DESTINATION lib)
        install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/libexiv2-xmp.a DESTINATION lib)
    endif ()
endif ()
