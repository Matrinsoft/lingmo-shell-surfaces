import QtQuick 2.15
import Lingmo.Shell 1.0

/**
 * DesktopSurface — full-screen desktop/wallpaper layer for Lingmo Shell.
 *
 * Wraps DesktopSurfaceItem (C++ LingmoShellSurfaces::DesktopSurface) and
 * renders the wallpaper image or solid colour on the Background layer.
 *
 * Usage:
 *   DesktopSurface {
 *       wallpaperSource: "file:///usr/share/lingmo/wallpapers/default.jpg"
 *       fillMode: LingmoShellSurfaces.PreserveAspectCrop
 *   }
 */
Item {
    id: root

    // ── Public API ──────────────────────────────────────────────────────────
    property url    wallpaperSource: ""
    property color  wallpaperColor:  "#1a1a2e"
    property int    fillMode:        Image.PreserveAspectCrop

    // Emitted when the user right-clicks the desktop
    signal contextMenuRequested(real x, real y)

    // ── Geometry ────────────────────────────────────────────────────────────
    anchors.fill: parent

    // ── Backend ─────────────────────────────────────────────────────────────
    DesktopSurfaceItem {
        id: backend
        wallpaperSource: root.wallpaperSource
        wallpaperColor:  root.wallpaperColor
        fillMode:        root.fillMode

        onContextMenuRequested: (x, y) => root.contextMenuRequested(x, y)
    }

    // ── Wallpaper rendering ─────────────────────────────────────────────────
    Rectangle {
        id: colorBackground
        anchors.fill: parent
        color: root.wallpaperColor
        visible: root.wallpaperSource.toString() === ""
    }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source:    root.wallpaperSource
        fillMode:  root.fillMode
        cache:     true
        asynchronous: true
        visible:   root.wallpaperSource.toString() !== ""
    }

    // ── Right-click handler ─────────────────────────────────────────────────
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: (mouse) => root.contextMenuRequested(mouse.x, mouse.y)
    }

    // ── Lifecycle ───────────────────────────────────────────────────────────
    Component.onCompleted: backend.show()

    function show()   { backend.show()   }
    function hide()   { backend.hide()   }
    function reload() { backend.reload() }
}
