install(DIRECTORY
        ${EXTERNAL_INSTALL_LOCATION}/include/exiv2 DESTINATION include
        FILES_MATCHING PATTERN *.h PATTERN *.hpp
        )

if (WIN32)
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/exiv2.lib DESTINATION lib)
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/exiv2-xmp.lib DESTINATION lib)
else()
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/libexiv2.a DESTINATION lib)
    install(FILES   ${EXTERNAL_INSTALL_LOCATION}/lib/libexiv2-xmp.a DESTINATION lib)
endif ()
