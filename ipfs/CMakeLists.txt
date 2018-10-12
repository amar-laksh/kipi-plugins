#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

subdirs(icons)

add_definitions(-DTRANSLATION_DOMAIN=\"kipiplugin_ipfs\")

set(kipiplugin_ipfs_PART_SRCS
    plugin_ipfs.cpp
    ipfsglobaluploadapi.cpp
    ipfswindow.cpp
    ipfsimageslist.cpp
   )

add_library(kipiplugin_ipfs MODULE ${kipiplugin_ipfs_PART_SRCS})

target_link_libraries(kipiplugin_ipfs
                      Qt5::Network

                      KF5::Kipi
                      KF5::WindowSystem
                      KF5::I18n

                      KF5kipiplugins
)

configure_file(kipiplugin_ipfs.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_ipfs.desktop)

install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_ipfs.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kipiplugin_ipfs                                     DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   kipiplugin_ipfsui.rc                                DESTINATION ${KXMLGUI_INSTALL_DIR}/kipi)
