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
find_package(LingmoUtils   REQUIRED)
find_package(LingmoConfig  REQUIRED)
find_package(LingmoTheme   REQUIRED)
if(TARGET Lingmo::Theme)
    set(LINGMO_THEME_TARGET Lingmo::Theme)
elseif(TARGET Lingmo::LingmoTheme)
    set(LINGMO_THEME_TARGET Lingmo::LingmoTheme)
else()
    message(FATAL_ERROR "LingmoTheme installed but defines neither Lingmo::Theme nor Lingmo::LingmoTheme")
endif()
message(STATUS "Found LingmoTheme target: ${LINGMO_THEME_TARGET}")

find_package(LingmoIPC       REQUIRED)
find_package(LingmoPlugin    REQUIRED)
find_package(LingmoShellCore REQUIRED)
if(TARGET Lingmo::ShellCore)
    set(LINGMO_SHELLCORE_TARGET Lingmo::ShellCore)
elseif(TARGET Lingmo::LingmoShellCore)
    set(LINGMO_SHELLCORE_TARGET Lingmo::LingmoShellCore)
else()
    message(FATAL_ERROR "LingmoShellCore installed but defines neither Lingmo::ShellCore nor Lingmo::LingmoShellCore")
endif()
message(STATUS "Found LingmoShellCore target: ${LINGMO_SHELLCORE_TARGET}")
find_package(LingmoWindow    REQUIRED)
find_package(LingmoIcon      QUIET)

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
