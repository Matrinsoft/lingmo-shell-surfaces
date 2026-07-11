#pragma once
#include <LingmoShellSurfaces/WindowManagerBackend.h>
#include <QList>

namespace Lingmo {

// Test double — no D-Bus, no Wayland.
// Lets unit tests exercise OverviewSurface logic without a real compositor.
class MockWindowManagerBackend : public WindowManagerBackend
{
    Q_OBJECT
public:
    explicit MockWindowManagerBackend(QObject *parent = nullptr)
        : WindowManagerBackend(parent) {}

    // ── Seeding helpers ───────────────────────────────────────────────────
    void setWindows(const QList<WindowInfo> &list)
    {
        windows_ = list;
        Q_EMIT windowsChanged();
    }

    void setWorkspaces(const QList<WorkspaceInfo> &list, int currentIdx = 0)
    {
        workspaces_ = list;
        currentIdx_ = currentIdx;
        Q_EMIT workspacesChanged();
        Q_EMIT currentWorkspaceIndexChanged(currentIdx_);
    }

    void setAvailable(bool av)
    {
        available_ = av;
        Q_EMIT availabilityChanged(av);
    }

    // ── Recorded calls ────────────────────────────────────────────────────
    QStringList activatedWindows;
    QStringList closedWindows;
    QStringList minimizedWindows;
    QStringList maximizedWindows;
    QList<int>  switchedWorkspaces;

    // ── WindowManagerBackend interface ────────────────────────────────────
    [[nodiscard]] QList<WindowInfo>    windows()             const override { return windows_; }
    [[nodiscard]] QList<WorkspaceInfo> workspaces()          const override { return workspaces_; }
    [[nodiscard]] int  currentWorkspaceIndex()               const override { return currentIdx_; }
    [[nodiscard]] bool isAvailable()                         const override { return available_; }

    void activateWindow(const QString &id) override
    {
        activatedWindows.append(id);
    }

    void closeWindow(const QString &id) override
    {
        closedWindows.append(id);
    }

    void minimizeWindow(const QString &id) override
    {
        minimizedWindows.append(id);
    }

    void maximizeWindow(const QString &id) override
    {
        maximizedWindows.append(id);
    }

    void switchToWorkspace(int idx) override
    {
        switchedWorkspaces.append(idx);
        currentIdx_ = idx;
        Q_EMIT currentWorkspaceIndexChanged(idx);
    }

    void initialize() override
    {
        available_ = true;
        Q_EMIT availabilityChanged(true);
    }

private:
    QList<WindowInfo>    windows_;
    QList<WorkspaceInfo> workspaces_;
    int                  currentIdx_ = 0;
    bool                 available_  = false;
};

} // namespace Lingmo
