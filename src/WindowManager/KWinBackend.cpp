#include "KWinBackend.h"
#include "VirtualDesktopIntegration.h"

#ifdef LINGMO_HAVE_WAYLAND
#include "ForeignToplevel/ForeignToplevelManager.h"
#include "ForeignToplevel/ForeignToplevelHandle.h"
#endif

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcKWin, "lingmo.shell.surfaces.kwin")

namespace Lingmo {

KWinBackend::KWinBackend(QObject *parent)
    : WindowManagerBackend(parent)
{
}

KWinBackend::~KWinBackend() = default;

// ── Initialisation ────────────────────────────────────────────────────────────

void KWinBackend::initialize()
{
    // ── KWin main D-Bus interface (for activateWindow / closeWindow) ──────────
    kwinInterface = new QDBusInterface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/KWin"),
        QStringLiteral("org.kde.KWin"),
        QDBusConnection::sessionBus(),
        this);

    if (!kwinInterface->isValid()) {
        qCWarning(lcKWin) << "KWin D-Bus interface not available:"
                          << kwinInterface->lastError().message();
        Q_EMIT availabilityChanged(false);
        return;
    }

    // ── Virtual desktop integration ───────────────────────────────────────────
    vdIntegration = new VirtualDesktopIntegration(this);
    connect(vdIntegration, &VirtualDesktopIntegration::ready, this, [this]() {
        rebuildWorkspaceList();
        available = true;
        Q_EMIT availabilityChanged(true);
        qCInfo(lcKWin) << "KWin backend ready:"
                       << workspaceList.size() << "desktops";
    });
    connect(vdIntegration, &VirtualDesktopIntegration::currentChanged, this,
            [this](int idx) {
                currentWsIndex = idx;
                // Refresh workspace list to flip active flags
                rebuildWorkspaceList();
                Q_EMIT currentWorkspaceIndexChanged(idx);
            });
    connect(vdIntegration, &VirtualDesktopIntegration::listChanged, this, [this]() {
        rebuildWorkspaceList();
    });
    vdIntegration->initialize();

#ifdef LINGMO_HAVE_WAYLAND
    // ── Foreign Toplevel manager (window list) ────────────────────────────────
    toplevelManager = new ForeignToplevelManager(this);
    connect(toplevelManager, &ForeignToplevelManager::handleAdded, this,
            [this](ForeignToplevelHandle *) {
                rebuildWindowList();
            });
    connect(toplevelManager, &ForeignToplevelManager::handleRemoved, this,
            [this](const QString &) {
                rebuildWindowList();
            });
    connect(toplevelManager, &ForeignToplevelManager::listStabilised, this, [this]() {
        rebuildWindowList();
        qCInfo(lcKWin) << "Initial window list ready:" << windowList.size() << "windows";
    });
    // ForeignToplevelManager binds automatically when the Wayland registry
    // advertises zwlr_foreign_toplevel_manager_v1.
#else
    qCInfo(lcKWin) << "Wayland not available — window list will be empty";
#endif
}

bool KWinBackend::isAvailable() const
{
    return available;
}

// ── Data rebuild ──────────────────────────────────────────────────────────────

void KWinBackend::rebuildWindowList()
{
#ifdef LINGMO_HAVE_WAYLAND
    if (!toplevelManager) {
        windowList.clear();
        Q_EMIT windowsChanged();
        return;
    }

    const QList<ForeignToplevelHandle *> rawHandles = toplevelManager->handles();
    const int currentWs = currentWsIndex;

    windowList.clear();
    windowList.reserve(rawHandles.size());

    for (ForeignToplevelHandle *h : rawHandles) {
        WindowInfo w;
        w.windowId       = h->handleId();
        w.title          = h->title();
        w.appId          = h->appId();
        w.isMinimized    = h->isMinimized();
        w.isActive       = h->isActive();
        // Foreign toplevel v1/v2/v3 does not carry workspace info directly;
        // all windows are associated with the current workspace for display.
        // A future protocol revision or KWin-specific extension could improve this.
        w.workspaceIndex = currentWs;
        // geometry is not available from this protocol; leave as default QRect()
        windowList.append(w);
    }

    qCDebug(lcKWin) << "Window list rebuilt:" << windowList.size() << "entries";
    Q_EMIT windowsChanged();
#else
    windowList.clear();
    Q_EMIT windowsChanged();
#endif
}

void KWinBackend::rebuildWorkspaceList()
{
    if (!vdIntegration) {
        workspaceList.clear();
        Q_EMIT workspacesChanged();
        return;
    }

    const QList<DesktopInfo> desktops = vdIntegration->desktops();
    currentWsIndex = vdIntegration->currentIndex();

    workspaceList.clear();
    workspaceList.reserve(desktops.size());

    for (int i = 0; i < desktops.size(); ++i) {
        const DesktopInfo &d = desktops.at(i);
        WorkspaceInfo ws;
        ws.workspaceIndex = i;
        ws.name           = d.name.isEmpty()
                            ? QStringLiteral("Desktop %1").arg(i + 1)
                            : d.name;
        ws.windowCount    = 0; // updated in rebuildWindowList when needed
        ws.active         = d.active;
        workspaceList.append(ws);
    }

    qCDebug(lcKWin) << "Workspace list rebuilt:" << workspaceList.size()
                    << "desktops; current index" << currentWsIndex;
    Q_EMIT workspacesChanged();
    Q_EMIT currentWorkspaceIndexChanged(currentWsIndex);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

QList<WindowInfo> KWinBackend::windows() const
{
    return windowList;
}

QList<WorkspaceInfo> KWinBackend::workspaces() const
{
    return workspaceList;
}

int KWinBackend::currentWorkspaceIndex() const
{
    return currentWsIndex;
}

// ── Actions ───────────────────────────────────────────────────────────────────

void KWinBackend::activateWindow(const QString &windowId)
{
#ifdef LINGMO_HAVE_WAYLAND
    if (toplevelManager) {
        const auto handles = toplevelManager->handles();
        for (ForeignToplevelHandle *h : handles) {
            if (h->handleId() == windowId) {
                h->requestActivate();
                return;
            }
        }
    }
#endif
    // Fallback: KWin D-Bus (older compositors)
    if (available && kwinInterface)
        kwinInterface->call(QStringLiteral("activateWindow"), windowId);
}

void KWinBackend::closeWindow(const QString &windowId)
{
#ifdef LINGMO_HAVE_WAYLAND
    if (toplevelManager) {
        const auto handles = toplevelManager->handles();
        for (ForeignToplevelHandle *h : handles) {
            if (h->handleId() == windowId) {
                h->requestClose();
                return;
            }
        }
    }
#endif
    if (available && kwinInterface)
        kwinInterface->call(QStringLiteral("closeWindow"), windowId);
}

void KWinBackend::minimizeWindow(const QString &windowId)
{
#ifdef LINGMO_HAVE_WAYLAND
    if (toplevelManager) {
        const auto handles = toplevelManager->handles();
        for (ForeignToplevelHandle *h : handles) {
            if (h->handleId() == windowId) {
                h->requestMinimize();
                return;
            }
        }
    }
#endif
    // No standard KWin D-Bus minimize call — ignore gracefully.
    Q_UNUSED(windowId)
}

void KWinBackend::maximizeWindow(const QString &windowId)
{
#ifdef LINGMO_HAVE_WAYLAND
    if (toplevelManager) {
        const auto handles = toplevelManager->handles();
        for (ForeignToplevelHandle *h : handles) {
            if (h->handleId() == windowId) {
                h->requestMaximize();
                return;
            }
        }
    }
#endif
    Q_UNUSED(windowId)
}

void KWinBackend::switchToWorkspace(int workspaceIndex)
{
    if (!vdIntegration || !vdIntegration->isValid()) {
        // Fallback to numeric D-Bus method
        if (available && kwinInterface) {
            kwinInterface->call(QStringLiteral("setCurrentDesktop"),
                                workspaceIndex + 1);
            currentWsIndex = workspaceIndex;
            Q_EMIT currentWorkspaceIndexChanged(workspaceIndex);
        }
        return;
    }

    const QList<DesktopInfo> desktops = vdIntegration->desktops();
    if (workspaceIndex < 0 || workspaceIndex >= desktops.size()) {
        qCWarning(lcKWin) << "switchToWorkspace: index out of range" << workspaceIndex;
        return;
    }

    // Use the VirtualDesktopManager's setCurrent(id) call.
    const QString desktopId = desktops.at(workspaceIndex).id;
    QDBusInterface vdIface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/VirtualDesktopManager"),
        QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
        QDBusConnection::sessionBus());
    if (vdIface.isValid())
        vdIface.call(QStringLiteral("setCurrent"), desktopId);
    else if (available && kwinInterface)
        kwinInterface->call(QStringLiteral("setCurrentDesktop"), workspaceIndex + 1);
}

} // namespace Lingmo
