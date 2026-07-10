import QtQuick 2.15
import QtQuick.Layouts 1.15
import Lingmo.Shell 1.0

/**
 * OverviewSurface — window overview / task switcher for Lingmo Shell.
 *
 * Wraps OverviewSurfaceItem (C++ LingmoShellSurfaces::OverviewSurface) and
 * provides a QML-friendly task overview with window thumbnails and workspace
 * switcher. Renders on the Overlay layer (above everything else).
 *
 * Usage:
 *   OverviewSurface {
 *       id: overview
 *   }
 *   // Toggle from a keyboard shortcut:
 *   overview.toggle()
 */
Item {
    id: root

    // ── Public API ──────────────────────────────────────────────────────────
    readonly property bool active: backend.visible
    readonly property bool ready:  backend.ready

    // ── Geometry ────────────────────────────────────────────────────────────
    anchors.fill: parent

    // ── Backend ─────────────────────────────────────────────────────────────
    OverviewSurfaceItem {
        id: backend
    }

    // ── Background blur overlay ─────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: root.active
        Behavior on opacity { NumberAnimation { duration: 180 } }
    }

    // ── Window grid ─────────────────────────────────────────────────────────
    GridView {
        id: windowGrid
        anchors {
            top: workspaceBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 32
        }
        cellWidth:  280
        cellHeight: 200
        model: backend.windowModel
        visible: root.active

        delegate: Item {
            width:  windowGrid.cellWidth  - 8
            height: windowGrid.cellHeight - 8

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#22ffffff"
                border.color: "#44ffffff"
                border.width: 1

                // Window title
                Text {
                    anchors {
                        bottom: parent.bottom
                        left:   parent.left
                        right:  parent.right
                        margins: 6
                    }
                    text: model.title ?? ""
                    color: "white"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: backend.activateWindow(model.windowId)
                }
            }
        }
    }

    // ── Workspace bar ────────────────────────────────────────────────────────
    RowLayout {
        id: workspaceBar
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: 16
        }
        spacing: 8
        visible: root.active

        Repeater {
            model: backend.workspaceModel
            delegate: Rectangle {
                width:  80
                height: 50
                radius: 6
                color:  model.active ? "#5555ff" : "#33ffffff"

                Text {
                    anchors.centerIn: parent
                    text: model.name ?? (index + 1).toString()
                    color: "white"
                    font.pixelSize: 12
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: backend.switchToWorkspace(model.workspaceId)
                }
            }
        }
    }

    // ── Keyboard handler ────────────────────────────────────────────────────
    Keys.onEscapePressed: hide()

    // ── Lifecycle ───────────────────────────────────────────────────────────
    Component.onCompleted: {
        // Overview starts hidden; show/hide driven by toggle()
    }

    function show()   { backend.show();  root.forceActiveFocus() }
    function hide()   { backend.hide()   }
    function toggle() { active ? hide() : show() }
    function reload() { backend.reload() }
}
