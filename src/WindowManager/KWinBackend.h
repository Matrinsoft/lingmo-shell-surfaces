#pragma once
#include "WindowManagerBackend.h"

class QDBusInterface;

namespace Lingmo {

class VirtualDesktopIntegration;

#ifdef LINGMO_HAVE_WAYLAND
class ForeignToplevelManager;
class ForeignToplevelHandle;
#endif

// KWin backend:
//   - Virtual desktops via org.kde.KWin.VirtualDesktopManager (D-Bus)
//   - Window list via zwlr_foreign_toplevel_management_v1 (Wayland, preferred)
//     with fallback to an empty list when not running under Wayland.
//
// Internal — not exported.
class KWinBackend : public WindowManagerBackend
{
    Q_OBJECT
public:
    explicit KWinBackend(QObject *parent = nullptr);
    ~KWinBackend() override;

    [[nodiscard]] QList<WindowInfo>    windows()             const override;
    [[nodiscard]] QList<WorkspaceInfo> workspaces()          const override;
    [[nodiscard]] int  currentWorkspaceIndex()               const override;
    [[nodiscard]] bool isAvailable()                         const override;

    void activateWindow(const QString &windowId)   override;
    void closeWindow(const QString &windowId)      override;
    void switchToWorkspace(int workspaceIndex)     override;
    void initialize()                              override;

private:
    void rebuildWindowList();
    void rebuildWorkspaceList();

    // D-Bus handle for KWin main interface (window activation/close)
    QDBusInterface *kwinInterface = nullptr;

    // Virtual desktop data source
    VirtualDesktopIntegration *vdIntegration = nullptr;

#ifdef LINGMO_HAVE_WAYLAND
    // Wayland foreign toplevel data source
    ForeignToplevelManager *toplevelManager = nullptr;
#endif

    QList<WindowInfo>    windowList;
    QList<WorkspaceInfo> workspaceList;
    int                  currentWsIndex = 0;
    bool                 available      = false;
};

} // namespace Lingmo
