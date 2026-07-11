#pragma once
#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <QList>
#include <QObject>
#include <QRect>
#include <QString>
#include <QUrl>

namespace Lingmo {

struct LINGMOSHELLSURFACES_EXPORT WindowInfo {
    QString windowId;
    QString title;
    QString appId;
    QUrl    thumbnailUrl;   // reserved — not implemented, kept for API compat
    QRect   geometry;
    int     workspaceIndex = 0;
    bool    isMinimized    = false;
    bool    isActive       = false;
};

struct LINGMOSHELLSURFACES_EXPORT WorkspaceInfo {
    int     workspaceIndex = 0;
    QString name;
    int     windowCount    = 0;
    bool    active         = false;
};

// Abstract window-manager backend.  Concrete implementations talk to a
// specific compositor (KWin via D-Bus/Wayland, etc.).  Obtain an instance
// through the factory WindowManagerBackend::create().
class LINGMOSHELLSURFACES_EXPORT WindowManagerBackend : public QObject
{
    Q_OBJECT
public:
    explicit WindowManagerBackend(QObject *parent = nullptr);
    ~WindowManagerBackend() override;

    // Factory — returns the best available backend for the running environment.
    static WindowManagerBackend *create(QObject *parent = nullptr);

    [[nodiscard]] virtual QList<WindowInfo>    windows()             const = 0;
    [[nodiscard]] virtual QList<WorkspaceInfo> workspaces()          const = 0;
    [[nodiscard]] virtual int  currentWorkspaceIndex()               const = 0;
    [[nodiscard]] virtual bool isAvailable()                         const = 0;

    virtual void activateWindow(const QString &windowId)                   = 0;
    virtual void closeWindow(const QString &windowId)                      = 0;
    virtual void minimizeWindow(const QString &windowId)                   = 0;
    virtual void maximizeWindow(const QString &windowId)                   = 0;
    virtual void switchToWorkspace(int workspaceIndex)                     = 0;
    virtual void initialize()                                              = 0;

Q_SIGNALS:
    void availabilityChanged(bool available);
    void windowsChanged();
    void workspacesChanged();
    void currentWorkspaceIndexChanged(int index);
};

} // namespace Lingmo
