set(CMAKE_AUTOMOC ON)

set(QML_SOURCES
    Lingmo/Shell/LingmoShellPlugin.cpp
)

add_library(LingmoShellSurfacesQml MODULE ${QML_SOURCES})

target_link_libraries(LingmoShellSurfacesQml PRIVATE
    Lingmo::ShellSurfaces
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
)

set_target_properties(LingmoShellSurfacesQml PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/Lingmo/Shell"
    OUTPUT_NAME LingmoShellSurfacesPlugin
)

set(QML_FILES
    Lingmo/Shell/PanelSurface.qml
    Lingmo/Shell/DesktopSurface.qml
    Lingmo/Shell/OverviewSurface.qml
)

# Install the QML plugin and QML files
install(TARGETS LingmoShellSurfacesQml
    LIBRARY DESTINATION ${CMAKE_INSTALL_QMLDIR}/Lingmo/Shell
    COMPONENT runtime
)
install(FILES ${QML_FILES}
    DESTINATION ${CMAKE_INSTALL_QMLDIR}/Lingmo/Shell
    COMPONENT runtime
)
install(FILES Lingmo/Shell/qmldir
    DESTINATION ${CMAKE_INSTALL_QMLDIR}/Lingmo/Shell
    COMPONENT runtime
)
