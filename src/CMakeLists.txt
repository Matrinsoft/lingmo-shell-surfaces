set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/LingmoShellSurfaces.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/Types.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/ShellSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/PanelSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/DesktopSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/OverviewSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/WindowManagerBackend.h
)

set(PRIVATE_HEADERS
    private/ShellSurface_p.h
    private/PanelSurface_p.h
    private/DesktopSurface_p.h
    private/OverviewSurface_p.h
    Platform/SurfaceBackend.h
    Platform/Generic/GenericSurfaceBackend.h
    Platform/Wayland/WaylandSurfaceBackend.h
    Platform/X11/X11SurfaceBackend.h
    WindowManager/VirtualDesktopIntegration.h
    WindowManager/ForeignToplevel/ForeignToplevelHandle.h
    WindowManager/ForeignToplevel/ForeignToplevelManager.h
)

set(SOURCES
    Types.cpp
    ShellSurface.cpp
    Panel/PanelSurface.cpp
    Desktop/DesktopSurface.cpp
    Overview/OverviewSurface.cpp
    Platform/SurfaceBackend.cpp
    Platform/Generic/GenericSurfaceBackend.cpp
    WindowManager/WindowManagerBackend.cpp
    WindowManager/KWinBackend.cpp
    WindowManager/VirtualDesktopIntegration.cpp
)

if(LINGMO_HAVE_WAYLAND)
    list(APPEND SOURCES Platform/Wayland/WaylandSurfaceBackend.cpp)
    list(APPEND SOURCES Platform/Wayland/LayerShellManager.cpp)
    list(APPEND SOURCES Platform/Wayland/LayerSurface.cpp)
    list(APPEND SOURCES WindowManager/ForeignToplevel/ForeignToplevelHandle.cpp)
    list(APPEND SOURCES WindowManager/ForeignToplevel/ForeignToplevelManager.cpp)
endif()

if(LINGMO_HAVE_X11)
    list(APPEND SOURCES Platform/X11/X11SurfaceBackend.cpp)
    list(APPEND SOURCES
        WindowManager/X11/RealXcbInterface.cpp
        WindowManager/X11/X11WindowBackend.cpp
    )
endif()

set(CMAKE_AUTOMOC ON)

add_library(LingmoShellSurfaces SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(Lingmo::ShellSurfaces ALIAS LingmoShellSurfaces)

target_include_directories(LingmoShellSurfaces
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LingmoShellSurfaces
    PUBLIC
        ${LINGMO_THEME_TARGET}
        ${LINGMO_SHELLCORE_TARGET}
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::DBus
    PRIVATE
        ${LINGMO_UTILS_TARGET}
        ${LINGMO_CONFIG_TARGET}
        ${LINGMO_IPC_TARGET}
        ${LINGMO_PLUGIN_TARGET}
        Qt6::Qml
)

if(LINGMO_HAVE_WAYLAND)
    target_link_libraries(LingmoShellSurfaces PRIVATE Qt6::WaylandClient Wayland::Client)
    target_compile_definitions(LingmoShellSurfaces PRIVATE LINGMO_HAVE_WAYLAND=1)
    qt6_generate_wayland_protocol_client_sources(LingmoShellSurfaces
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/Platform/Wayland/Protocol/wlr-layer-shell-unstable-v1.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/Platform/Wayland/Protocol/wlr-foreign-toplevel-management-unstable-v1.xml
    )
    target_include_directories(LingmoShellSurfaces PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(LingmoShellSurfaces PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WindowManager/ForeignToplevel)
endif()

if(LINGMO_HAVE_X11)
    target_link_libraries(LingmoShellSurfaces PRIVATE ${X11_LIBRARIES} xcb)
    target_compile_definitions(LingmoShellSurfaces PRIVATE LINGMO_HAVE_X11=1)
    target_include_directories(LingmoShellSurfaces PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/WindowManager/X11)
endif()

target_compile_definitions(LingmoShellSurfaces PRIVATE LINGMOSHELLSURFACES_LIBRARY)

set_target_properties(LingmoShellSurfaces PROPERTIES
    SOVERSION 1
    VERSION   ${PROJECT_VERSION}
    DEFINE_SYMBOL LINGMOSHELLSURFACES_LIBRARY
    OUTPUT_NAME LingmoShellSurfaces
)

configure_file(
    "${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesExport.h.in"
    "${CMAKE_BINARY_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesExport.h"
)
configure_file(
    "${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesVersion.h.in"
    "${CMAKE_BINARY_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesVersion.h"
)

# ── Install ────────────────────────────────────────────────
install(TARGETS LingmoShellSurfaces
    EXPORT  LingmoShellSurfacesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}    COMPONENT runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}    COMPONENT runtime
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/LingmoShellSurfaces
    COMPONENT devel
    FILES_MATCHING PATTERN "*.h"
    PATTERN "*.h.in" EXCLUDE
)

install(FILES
    "${CMAKE_BINARY_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesExport.h"
    "${CMAKE_BINARY_DIR}/include/LingmoShellSurfaces/LingmoShellSurfacesVersion.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/LingmoShellSurfaces
    COMPONENT devel
)

install(EXPORT LingmoShellSurfacesTargets
    FILE      LingmoShellSurfacesTargets.cmake
    NAMESPACE Lingmo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoShellSurfaces
    COMPONENT devel
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/LingmoShellSurfacesConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/LingmoShellSurfacesConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoShellSurfaces
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/LingmoShellSurfacesConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_BINARY_DIR}/LingmoShellSurfacesConfig.cmake"
    "${CMAKE_BINARY_DIR}/LingmoShellSurfacesConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LingmoShellSurfaces
    COMPONENT devel
)
