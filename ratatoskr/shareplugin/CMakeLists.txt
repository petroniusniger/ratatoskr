# Shareplugin - ContentHub integration for sending files via Bluetooth

set(SHAREPLUGIN_NAME "shareplugin")

set(SHAREPLUGIN_SOURCES
    main.cpp
    bttransfer.cpp
    adapter.cpp
    devicenameresolver.cpp
)

set(SHAREPLUGIN_HEADERS
    bttransfer.h
    adapter.h
    devicenameresolver.h
)

# Shareplugin resources
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Main.qml ${CMAKE_CURRENT_BINARY_DIR}/Main.qml COPYONLY)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/shareplugin.qrc
"<RCC>
    <qresource prefix=\"/\">
        <file>Main.qml</file>
    </qresource>
</RCC>")

qt5_add_resources(SHAREPLUGIN_QT_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/shareplugin.qrc)

add_executable(${SHAREPLUGIN_NAME} ${SHAREPLUGIN_SOURCES} ${SHAREPLUGIN_HEADERS} ${SHAREPLUGIN_QT_RESOURCES})
set_target_properties(${SHAREPLUGIN_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
target_link_libraries(${SHAREPLUGIN_NAME} Qt5::Gui Qt5::Qml Qt5::Quick Qt5::DBus Qt5::Bluetooth)
install(TARGETS ${SHAREPLUGIN_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

# Install shareplugin config files
install(FILES shareplugin.apparmor DESTINATION /)
install(FILES shareplugin.desktop DESTINATION /)
install(FILES bluetooth-contenthub.json DESTINATION /)
install(FILES icon.svg DESTINATION shareplugin-assets)
