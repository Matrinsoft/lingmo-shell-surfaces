set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/LingmoShellSurfaces.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/Types.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/ShellSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/PanelSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/DesktopSurface.h
    ${CMAKE_SOURCE_DIR}/include/LingmoShellSurfaces/OverviewSurface.h
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
)

set(SOURCES
    Types.cpp
    ShellSurface.cpp
    Panel/PanelSurface.cpp
    Desktop/DesktopSurface.cpp
    Overview/OverviewSurface.cpp
    Platform/SurfaceBackend.cpp
    Platform/Generic/GenericSurfaceBackend.cpp
)

if(LINGMO_HAVE_WAYLAND)
    list(APPEND SOURCES Platform/Wayland/WaylandSurfaceBackend.cpp)
endif()

if(LINGMO_HAVE_X11)
    list(APPEND SOURCES Platform/X11/X11SurfaceBackend.cpp)
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
endif()

if(LINGMO_HAVE_X11)
    target_link_libraries(LingmoShellSurfaces PRIVATE ${X11_LIBRARIES})
    target_compile_definitions(LingmoShellSurfaces PRIVATE LINGMO_HAVE_X11=1)
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
