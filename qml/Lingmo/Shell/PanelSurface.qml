import QtQuick 2.15
import QtQuick.Layouts 1.15
import Lingmo.Shell 1.0

/**
 * PanelSurface — top-level Panel component for Lingmo Shell.
 *
 * Wraps PanelSurfaceItem (C++ LingmoShellSurfaces::PanelSurface) and
 * provides a QML-friendly API with a default horizontal RowLayout for
 * placing applets.
 *
 * Usage:
 *   PanelSurface {
 *       edge: LingmoShellSurfaces.Top
 *       panelHeight: 36
 *       autohide: false
 *       applets: [ClockApplet {}, LauncherApplet {}]
 *   }
 */
Item {
    id: root

    // ── Public API ──────────────────────────────────────────────────────────
    property int  edge:        LingmoShellSurfaces.Top
    property int  panelHeight: 36
    property bool autohide:    false
    property var  applets:     []

    // ── Geometry ────────────────────────────────────────────────────────────
    width:  (edge === LingmoShellSurfaces.Top || edge === LingmoShellSurfaces.Bottom)
            ? Screen.width : panelHeight
    height: (edge === LingmoShellSurfaces.Top || edge === LingmoShellSurfaces.Bottom)
            ? panelHeight  : Screen.height

    // ── Backend ─────────────────────────────────────────────────────────────
    PanelSurfaceItem {
        id: backend
        edge:        root.edge
        panelHeight: root.panelHeight
        autohide:    root.autohide
    }

    // ── Applet container ────────────────────────────────────────────────────
    RowLayout {
        id: appletContainer
        anchors.fill: parent
        spacing: 0
    }

    // ── Lifecycle ───────────────────────────────────────────────────────────
    Component.onCompleted: {
        backend.show()
        _syncApplets()
    }

    onAppletsChanged: _syncApplets()

    function _syncApplets() {
        // Reparent each applet item into the container
        for (var i = 0; i < applets.length; ++i) {
            var item = applets[i]
            if (item && item.parent !== appletContainer) {
                item.parent = appletContainer
                backend.appendApplet(item)
            }
        }
    }

    function show()   { backend.show()   }
    function hide()   { backend.hide()   }
    function reload() { backend.reload() }
}
