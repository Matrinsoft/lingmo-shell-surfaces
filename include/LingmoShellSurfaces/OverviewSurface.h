#pragma once

#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QAbstractListModel>
#include <QObject>

namespace Lingmo {

class OverviewSurfacePrivate;

// OverviewSurface — window overview / workspace switcher surface.
//
// Occupies the Overlay layer on Wayland, covering the entire screen when
// active.  Displays window thumbnails from KWin via D-Bus / KWin scripting
// API and provides a workspace grid.
//
// Window data flows in via D-Bus:
//   Interface : org.kde.KWin (or org.lingmo.KWin bridge when available)
//   Methods   : queryWindowList(), activateWindow(), closeWindow()
//
// The overview does NOT manage windows itself — it only reads state and
// forwards user actions back to KWin.
//
// Example (QML):
//   OverviewSurface {
//       onWindowActivated: (wid) => console.log("activate", wid)
//   }
class LINGMOSHELLSURFACES_EXPORT OverviewSurface : public ShellSurface
{
    Q_OBJECT

    Q_PROPERTY(QAbstractListModel *windowModel
               READ windowModel CONSTANT)
    Q_PROPERTY(QAbstractListModel *workspaceModel
               READ workspaceModel CONSTANT)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

public:
    explicit OverviewSurface(QObject *parent = nullptr);
    ~OverviewSurface() override;

    // Read-only model of currently open windows.
    // Roles: windowId (QString), title (QString), thumbnailUrl (QUrl),
    //        workspaceIndex (int), isMinimized (bool).
    [[nodiscard]] QAbstractListModel *windowModel() const;

    // Read-only model of virtual desktops / workspaces.
    // Roles: workspaceIndex (int), name (QString), windowCount (int).
    [[nodiscard]] QAbstractListModel *workspaceModel() const;

    // True once the D-Bus connection to KWin is established and the initial
    // window/workspace lists have been received.
    [[nodiscard]] bool isReady() const;

    SurfaceLayer layer() const override;

public Q_SLOTS:
    // Asks KWin to raise and focus the window with the given ID.
    void activateWindow(const QString &windowId);

    // Asks KWin to close the window with the given ID.
    void closeWindow(const QString &windowId);

    // Switches the active workspace to the given index.
    void switchToWorkspace(int workspaceIndex);

Q_SIGNALS:
    void readyChanged(bool ready);
    void windowActivated(const QString &windowId);
    void windowClosed(const QString &windowId);

protected:
    void createWindow() override;
};

} // namespace Lingmo
