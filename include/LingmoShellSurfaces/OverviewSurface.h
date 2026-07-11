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
// active.  Displays window thumbnails via the WindowManagerBackend and
// provides a workspace grid.
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
    Q_PROPERTY(bool ready
               READ isReady NOTIFY readyChanged)
    Q_PROPERTY(int currentWorkspace
               READ currentWorkspace NOTIFY currentWorkspaceChanged)

public:
    explicit OverviewSurface(QObject *parent = nullptr);
    ~OverviewSurface() override;

    // Read-only model of currently open windows.
    // Roles: windowId, title, thumbnailUrl, workspaceIndex, isMinimized.
    [[nodiscard]] QAbstractListModel *windowModel() const;

    // Read-only model of virtual desktops / workspaces.
    // Roles: workspaceIndex, name, windowCount, active.
    [[nodiscard]] QAbstractListModel *workspaceModel() const;

    // True once the backend connection is established.
    [[nodiscard]] bool isReady() const;

    // Index of the currently active workspace (0-based).
    [[nodiscard]] int currentWorkspace() const;

    SurfaceLayer layer() const override;

public Q_SLOTS:
    void activateWindow(const QString &windowId);
    void closeWindow(const QString &windowId);
    void switchToWorkspace(int workspaceIndex);

Q_SIGNALS:
    void readyChanged(bool ready);
    void windowActivated(const QString &windowId);
    void windowClosed(const QString &windowId);
    void currentWorkspaceChanged(int index);

protected:
    void createWindow() override;
};

} // namespace Lingmo
