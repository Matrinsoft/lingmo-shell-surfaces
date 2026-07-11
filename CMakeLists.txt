cmake_minimum_required(VERSION 3.21)

cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0092 NEW)
cmake_policy(SET CMP0128 NEW)

project(LingmoShellSurfaces
    VERSION 1.0.0
    DESCRIPTION "Lingmo Desktop - Shell Surface Layer (Panel, Desktop, Overview)"
    HOMEPAGE_URL "https://lingmo.org"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LingmoCompilerSettings)
include(LingmoSanitizers)
include(LingmoPCH)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(BUILD_TESTING           "Build tests"           ON)
option(LINGMO_ENABLE_SANITIZERS "Enable sanitizers"    OFF)
option(LINGMO_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
option(BUILD_EXAMPLES          "Build example programs" OFF)

# ── Qt dependencies ───────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml DBus)
find_package(Qt6 COMPONENTS Test QUIET)
qt_policy(SET QTP0001 NEW)

# ── Lingmo dependencies ───────────────────────────────────
# Helper: resolve build-tree alias (Lingmo::X) vs installed target (Lingmo::LingmoX)
macro(lingmo_resolve_target PKG SHORT FULL)
    find_package(${PKG} REQUIRED)
    if(TARGET Lingmo::${SHORT})
        set(LINGMO_${SHORT}_TARGET Lingmo::${SHORT})
    elseif(TARGET Lingmo::${FULL})
        set(LINGMO_${SHORT}_TARGET Lingmo::${FULL})
    else()
        message(FATAL_ERROR "${PKG}: neither Lingmo::${SHORT} nor Lingmo::${FULL} found")
    endif()
    message(STATUS "Found ${PKG} target: ${LINGMO_${SHORT}_TARGET}")
endmacro()

lingmo_resolve_target(LingmoUtils    Utils      LingmoUtils)
lingmo_resolve_target(LingmoConfig   Config     LingmoConfig)
lingmo_resolve_target(LingmoIPC      IPC        LingmoIPC)
lingmo_resolve_target(LingmoPlugin   Plugin     LingmoPlugin)
lingmo_resolve_target(LingmoWindow   Window     LingmoWindow)
lingmo_resolve_target(LingmoShellCore ShellCore LingmoShellCore)

find_package(LingmoTheme REQUIRED)
if(TARGET Lingmo::Theme)
    set(LINGMO_THEME_TARGET Lingmo::Theme)
elseif(TARGET Lingmo::LingmoTheme)
    set(LINGMO_THEME_TARGET Lingmo::LingmoTheme)
else()
    message(FATAL_ERROR "LingmoTheme: neither Lingmo::Theme nor Lingmo::LingmoTheme found")
endif()
message(STATUS "Found LingmoTheme target: ${LINGMO_THEME_TARGET}")

find_package(LingmoIcon QUIET)

# ── Wayland detection ─────────────────────────────────────
find_package(Qt6 COMPONENTS WaylandClient QUIET)
find_package(Wayland COMPONENTS Client QUIET)
if(Qt6WaylandClient_FOUND AND Wayland_Client_FOUND)
    set(LINGMO_HAVE_WAYLAND ON)
    message(STATUS "Wayland support: ON")
else()
    set(LINGMO_HAVE_WAYLAND OFF)
    message(STATUS "Wayland support: OFF (need Qt6::WaylandClient + wayland-client)")
endif()

# ── X11 detection ─────────────────────────────────────────
find_package(X11 QUIET)
find_package(Qt6 COMPONENTS X11Extras QUIET)
if(X11_FOUND)
    set(LINGMO_HAVE_X11 ON)
    message(STATUS "X11 support: ON")
else()
    set(LINGMO_HAVE_X11 OFF)
    message(STATUS "X11 support: OFF")
endif()

# ── Subdirectories ────────────────────────────────────────
add_subdirectory(src)

if(Qt6Qml_FOUND AND Qt6Quick_FOUND)
    add_subdirectory(qml)
endif()

if(BUILD_EXAMPLES AND Qt6Quick_FOUND)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING AND Qt6Test_FOUND)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── CPack ─────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME    "lingmo-shell-surfaces")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR  "Lingmo Project")
set(CPACK_GENERATOR       "DEB;RPM")
include(CPack)
