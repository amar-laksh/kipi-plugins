#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_definitions(-DTRANSLATION_DOMAIN=\"kipiplugin_kmlexport\")

set(kipiplugin_kmlexport_PART_SRCS plugin_kmlexport.cpp
                                   gpsdataparser.cpp
                                   kmlexport.cpp
                                   kmlgpsdataparser.cpp
                                   kmlexportconfig.cpp
)

add_library(kipiplugin_kmlexport MODULE ${kipiplugin_kmlexport_PART_SRCS})

target_link_libraries(kipiplugin_kmlexport
                      KF5::Parts
                      KF5::KDELibs4Support
                      
                      ${KF5Kipi_LIBRARIES}
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}

                      KF5kipiplugins
                     )

configure_file(kipiplugin_kmlexport.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_kmlexport.desktop)

install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_kmlexport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kipiplugin_kmlexport                                     DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   kipiplugin_kmlexportui.rc                                DESTINATION ${KXMLGUI_INSTALL_DIR}/kipi)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()