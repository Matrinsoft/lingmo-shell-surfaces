# lingmo-shell-surfaces

Shell surface layer for [Lingmo Desktop](https://lingmo.org) — provides **Panel**, **Desktop**, and **Overview** surfaces built on top of `wlr-layer-shell-unstable-v1` (Wayland) and EWMH override-redirect windows (X11).

## Overview

`lingmo-shell-surfaces` is a Qt 6 shared library that abstracts the platform-specific mechanics of anchoring shell windows to screen edges or covering the entire screen. It sits between the compositor protocol layer and the QML shell UI.

```
┌─────────────────────────────────────────────────────┐
│                   QML Shell UI                      │
│   PanelSurface.qml  DesktopSurface.qml  Overview…  │
├─────────────────────────────────────────────────────┤
│            LingmoShellSurfaces (this lib)           │
│   PanelSurface   DesktopSurface   OverviewSurface   │
│              ShellSurface (base)                    │
├──────────────────────┬──────────────────────────────┤
│  WaylandSurface-     │  X11SurfaceBackend           │
│  Backend             │  (_NET_WM_STRUT_PARTIAL)     │
│  (zwlr_layer_shell)  │                              │
└──────────────────────┴──────────────────────────────┘
```

## Features

- **PanelSurface** — top/bottom/left/right panel anchored to a screen edge; reserves an exclusive zone so tiled windows avoid it; supports autohide.
- **DesktopSurface** — full-screen wallpaper layer rendered below all windows; emits `contextMenuRequested` for shell plugins.
- **OverviewSurface** — full-screen window overview / workspace switcher on the Overlay layer; integrates with KWin via D-Bus.
- **Automatic platform detection** — picks Wayland layer-shell, X11 EWMH, or generic offscreen backend at runtime.
- **QML plugin** — exposes all surface types as `Lingmo.Shell 1.0` QML types.
- **PIMPL** — stable ABI with `SameMajorVersion` compatibility policy.

## Requirements

| Dependency | Version |
|---|---|
| CMake | ≥ 3.21 |
| Qt | 6.4+ (Core, Gui, Quick, Qml, DBus) |
| Qt WaylandClient | optional — enables Wayland backend |
| libwayland-client | optional — required alongside Qt WaylandClient |
| libX11 | optional — enables X11 backend |
| LingmoUtils | latest |
| LingmoConfig | latest |
| LingmoIPC | latest |
| LingmoPlugin | latest |
| LingmoTheme | latest |
| LingmoWindow | latest |
| LingmoShellCore | latest |
| LingmoIcon | optional |

## Building

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_TESTING=ON
cmake --build build --parallel
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `BUILD_TESTING` | `ON` | Build unit tests |
| `LINGMO_ENABLE_SANITIZERS` | `OFF` | Enable AddressSanitizer |
| `LINGMO_ENABLE_UNITY_BUILD` | `OFF` | Enable unity (jumbo) builds |
| `BUILD_EXAMPLES` | `OFF` | Build example programs |

## Installing

```bash
cmake --install build
```

Headers are installed to `$prefix/include/LingmoShellSurfaces/`.  
The CMake package config is at `$libdir/cmake/LingmoShellSurfaces/`.

## Usage

### C++

```cpp
#include <LingmoShellSurfaces/PanelSurface.h>
#include <LingmoShellSurfaces/DesktopSurface.h>

using namespace Lingmo;

// Create a top panel
auto *panel = new PanelSurface(this);
panel->setEdge(SurfaceEdge::Top);
panel->setPanelHeight(36);
panel->show();

// Create a wallpaper layer
auto *desktop = new DesktopSurface(this);
desktop->setWallpaperSource(QStringLiteral("file:///usr/share/lingmo/wallpapers/default.jpg"));
desktop->show();
```

### CMake (consumer)

```cmake
find_package(LingmoShellSurfaces REQUIRED)
target_link_libraries(my_app PRIVATE Lingmo::ShellSurfaces)
```

### QML

```qml
import Lingmo.Shell 1.0

PanelSurface {
    edge: LingmoShellSurfaces.Top
    panelHeight: 36
    autohide: false
}

DesktopSurface {
    wallpaperSource: "file:///usr/share/lingmo/wallpapers/default.jpg"
    fillMode: Image.PreserveAspectCrop
}

OverviewSurface {
    id: overview
}
// Toggle from a keyboard shortcut:
// overview.toggle()
```

## API

### `ShellSurface` (abstract base)

| Member | Description |
|---|---|
| `show()` | Map the surface on its assigned screen |
| `hide()` | Unmap without destroying |
| `reload()` | Destroy and recreate the native window |
| `isVisible()` | Whether the surface is currently mapped |
| `screen()` / `setScreen()` | Screen assignment |
| `layer()` | The Wayland layer (constant per subclass) |
| `platform()` | Detected platform: Wayland / X11 / Offscreen |

### `PanelSurface`

| Member | Description |
|---|---|
| `edge` | `SurfaceEdge::Top/Bottom/Left/Right` |
| `panelHeight` | Pixel thickness (default 36) |
| `autohide` | Slide off-screen when not hovered |
| `exclusiveZone` | Reserved space reported to compositor (read-only) |
| `appendApplet(QObject*)` | Add an applet to the panel |
| `removeApplet(QObject*)` / `removeApplet(int)` | Remove an applet |

### `DesktopSurface`

| Member | Description |
|---|---|
| `wallpaperSource` | `file://` or `qrc://` URI of the wallpaper image |
| `wallpaperColor` | `QColor` fallback when image is absent |
| `fillMode` | `Stretch / PreserveAspectFit / PreserveAspectCrop / Tile` |
| `contextMenuRequested(QPoint)` | Emitted on right-click |

### `OverviewSurface`

| Member | Description |
|---|---|
| `windowModel()` | `QAbstractListModel` — open windows (title, windowId, …) |
| `workspaceModel()` | `QAbstractListModel` — virtual desktops |
| `isReady()` | `true` once KWin D-Bus connection is established |
| `activateWindow(QString)` | Raise and focus a window by ID |
| `closeWindow(QString)` | Close a window by ID |
| `switchToWorkspace(int)` | Activate a virtual desktop by index |

## Platform notes

### Wayland

Uses `zwlr_layer_shell_v1` (wlroots layer-shell protocol). The compositor must advertise this protocol — KWin 5.27+, Wayfire, Hyprland, and most wlroots-based compositors do. The backend is selected automatically when `Qt::WaylandClient` is available and the protocol is advertised; otherwise falls back to the generic backend.

### X11

Uses override-redirect windows with `_NET_WM_WINDOW_TYPE` and `_NET_WM_STRUT_PARTIAL` EWMH hints. Panel exclusive zones are enforced by communicating the strut to the window manager.

### Offscreen / Testing

A generic `QWindow`-based backend is used on headless platforms. All unit tests run against this backend via `QT_QPA_PLATFORM=offscreen`.

## Testing

```bash
ctest --test-dir build --output-on-failure
```

| Test | What it covers |
|---|---|
| `tst_ShellSurface` | Enum values, `SurfaceGeometry`, debug output |
| `tst_PanelSurface` | Properties, signals, applet management, exclusiveZone |
| `tst_DesktopSurface` | Wallpaper properties, fill mode, layer constant |
| `tst_OverviewSurface` | Layer constant, models non-null, role names |
| `tst_SurfaceBackend` | show/hide/reload lifecycle, idempotency, platform detection |
| `tst_QmlImport` | QML type instantiation, enum access from QML |

## Packaging

### Debian / Ubuntu

```bash
dpkg-buildpackage -us -uc -b
```

Produces three packages: `liblingmoshellsurfaces1`, `liblingmoshellsurfaces-dev`, `qml-module-lingmo-shell`.

### Fedora / RPM

```bash
rpmbuild -ba rpm/lingmo-shell-surfaces.spec
```

## CI

Four GitHub Actions workflows:

| Workflow | Triggers | What it does |
|---|---|---|
| `build.yml` | push / PR | GCC build on Debian trixie |
| `test.yml` | push / PR | GCC + Clang build + `ctest` |
| `package.yml` | release / manual | Builds `.deb` (Debian trixie) and `.rpm` (Fedora 44) |
| `abi.yml` | push to main / PR | ABI compliance check with `abi-compliance-checker` |

## Architecture decisions

**Why a separate library instead of part of lingmo-shell-core?**  
Shell surfaces need platform-specific code (Wayland protocol bindings, X11 Xlib calls) that would pull heavy optional dependencies into the core library. Keeping them separate allows the shell to load the surface library only on supported platforms.

**Why PIMPL on all exported classes?**  
`SameMajorVersion` ABI compatibility — adding private members never breaks downstream binaries.

**Why Q_NAMESPACE + Q_ENUM_NS on the `Lingmo` namespace?**  
`enum class` values in a namespace cannot be exposed to QML without `staticMetaObject`. `Q_NAMESPACE_EXPORT` gives the namespace a `staticMetaObject` usable with `qmlRegisterUncreatableMetaObject`, exposing `SurfaceEdge`, `SurfaceLayer`, and `SurfacePlatform` under the `LingmoShellSurfaces` QML name.

## License

LGPL-2.1-or-later — see [LICENSES/LGPL-2.1-or-later.txt](LICENSES/LGPL-2.1-or-later.txt).
