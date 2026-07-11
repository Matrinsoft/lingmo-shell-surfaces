#include <LingmoShellSurfaces/OverviewSurface.h>
#include "private/OverviewSurface_p.h"
#include "Platform/SurfaceBackend.h"
#include "WindowManager/WindowManagerBackend.h"

#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcOverview, "lingmo.shell.surfaces.overview")

namespace Lingmo {

// ── WindowListModel ──────────────────────────────────────────────────────────

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WindowListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_windows.size());
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_windows.size()))
        return {};

    const QVariantMap &w = m_windows.at(index.row());
    switch (role) {
    case WindowIdRole:       return w.value(QStringLiteral("windowId"));
    case TitleRole:          return w.value(QStringLiteral("title"));
    case AppIdRole:          return w.value(QStringLiteral("appId"));
    case ThumbnailUrlRole:   return w.value(QStringLiteral("thumbnailUrl"));
    case WorkspaceIndexRole: return w.value(QStringLiteral("workspaceIndex"));
    case IsMinimizedRole:    return w.value(QStringLiteral("isMinimized"));
    case IsActiveRole:       return w.value(QStringLiteral("isActive"));
    default:                 return {};
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const
{
    return {
        { WindowIdRole,       "windowId"       },
        { TitleRole,          "title"          },
        { AppIdRole,          "appId"          },
        { ThumbnailUrlRole,   "thumbnailUrl"   },
        { WorkspaceIndexRole, "workspaceIndex" },
        { IsMinimizedRole,    "isMinimized"    },
        { IsActiveRole,       "isActive"       },
    };
}

void WindowListModel::setWindows(const QList<QVariantMap> &windows)
{
    beginResetModel();
    m_windows = windows;
    endResetModel();
}

// ── WorkspaceListModel ───────────────────────────────────────────────────────

WorkspaceListModel::WorkspaceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WorkspaceListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_workspaces.size());
}

QVariant WorkspaceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_workspaces.size()))
        return {};

    const QVariantMap &ws = m_workspaces.at(index.row());
    switch (role) {
    case WorkspaceIndexRole:
        return ws.value(QStringLiteral("workspaceIndex"));
    case NameRole:
        return ws.value(QStringLiteral("name"));
    case WindowCountRole:
        return ws.value(QStringLiteral("windowCount"));
    case ActiveRole:
        return ws.value(QStringLiteral("workspaceIndex")).toInt() == currentWorkspaceIndex;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkspaceListModel::roleNames() const
{
    return {
        { WorkspaceIndexRole, "workspaceIndex" },
        { NameRole,           "name"           },
        { WindowCountRole,    "windowCount"    },
        { ActiveRole,         "active"         },
    };
}

void WorkspaceListModel::setWorkspaces(const QList<QVariantMap> &workspaces)
{
    beginResetModel();
    m_workspaces = workspaces;
    endResetModel();
}

void WorkspaceListModel::setCurrentWorkspaceIndex(int idx)
{
    if (currentWorkspaceIndex == idx)
        return;

    const int oldIdx = currentWorkspaceIndex;
    currentWorkspaceIndex = idx;

    // Emit dataChanged for the two rows whose ActiveRole flipped.
    auto notifyRow = [this](int wsIndex) {
        for (int row = 0; row < static_cast<int>(m_workspaces.size()); ++row) {
            if (m_workspaces.at(row).value(QStringLiteral("workspaceIndex")).toInt() == wsIndex) {
                const QModelIndex mi = index(row);
                Q_EMIT dataChanged(mi, mi, { ActiveRole });
                break;
            }
        }
    };
    notifyRow(oldIdx);
    notifyRow(idx);
}

// ── OverviewSurfacePrivate ───────────────────────────────────────────────────

OverviewSurfacePrivate::OverviewSurfacePrivate(OverviewSurface *q)
    : ShellSurfacePrivate(q)
    , windowModel(new WindowListModel(q))
    , workspaceModel(new WorkspaceListModel(q))
{
}

void OverviewSurfacePrivate::connectToWindowManager()
{
    auto *ovq = static_cast<OverviewSurface *>(q);

    windowManager = WindowManagerBackend::create(ovq);

    QObject::connect(windowManager, &WindowManagerBackend::windowsChanged,
                     ovq, [this]() {
        const auto wins = windowManager->windows();
        QList<QVariantMap> list;
        list.reserve(wins.size());
        for (const WindowInfo &wi : wins) {
            list.append({
                { QStringLiteral("windowId"),       wi.windowId              },
                { QStringLiteral("title"),          wi.title                 },
                { QStringLiteral("appId"),          wi.appId                 },
                { QStringLiteral("thumbnailUrl"),   wi.thumbnailUrl          },
                { QStringLiteral("workspaceIndex"), wi.workspaceIndex        },
                { QStringLiteral("isMinimized"),    wi.isMinimized           },
                { QStringLiteral("isActive"),       wi.isActive              },
            });
        }
        windowModel->setWindows(list);
    });

    QObject::connect(windowManager, &WindowManagerBackend::workspacesChanged,
                     ovq, [this]() {
        const auto wss = windowManager->workspaces();
        QList<QVariantMap> list;
        list.reserve(wss.size());
        for (const WorkspaceInfo &ws : wss) {
            list.append({
                { QStringLiteral("workspaceIndex"), ws.workspaceIndex },
                { QStringLiteral("name"),           ws.name           },
                { QStringLiteral("windowCount"),    ws.windowCount    },
            });
        }
        workspaceModel->setWorkspaces(list);
        workspaceModel->setCurrentWorkspaceIndex(windowManager->currentWorkspaceIndex());
    });

    QObject::connect(windowManager, &WindowManagerBackend::currentWorkspaceIndexChanged,
                     ovq, [this, ovq](int idx) {
        workspaceModel->setCurrentWorkspaceIndex(idx);
        Q_EMIT ovq->currentWorkspaceChanged(idx);
    });

    QObject::connect(windowManager, &WindowManagerBackend::availabilityChanged,
                     ovq, [this, ovq](bool available) {
        if (available && !ready) {
            ready = true;
            Q_EMIT ovq->readyChanged(true);
            qCInfo(lcOverview) << "WindowManagerBackend available";
        } else if (!available && ready) {
            ready = false;
            Q_EMIT ovq->readyChanged(false);
        }
    });

    windowManager->initialize();
}

// ── OverviewSurface ──────────────────────────────────────────────────────────

OverviewSurface::OverviewSurface(QObject *parent)
    : ShellSurface(new OverviewSurfacePrivate(this), parent)
{
    LINGMO_D(OverviewSurface);
    QMetaObject::invokeMethod(this, [d] { d->connectToWindowManager(); },
                              Qt::QueuedConnection);
}

OverviewSurface::~OverviewSurface() = default;

QAbstractListModel *OverviewSurface::windowModel() const
{
    LINGMO_CD(OverviewSurface);
    return d->windowModel;
}

QAbstractListModel *OverviewSurface::workspaceModel() const
{
    LINGMO_CD(OverviewSurface);
    return d->workspaceModel;
}

bool OverviewSurface::isReady() const
{
    LINGMO_CD(OverviewSurface);
    return d->ready;
}

int OverviewSurface::currentWorkspace() const
{
    LINGMO_CD(OverviewSurface);
    return d->workspaceModel->currentWorkspaceIndex;
}

SurfaceLayer OverviewSurface::layer() const
{
    return SurfaceLayer::Overlay;
}

void OverviewSurface::activateWindow(const QString &windowId)
{
    LINGMO_D(OverviewSurface);
    if (!d->windowManager)
        return;
    d->windowManager->activateWindow(windowId);
    Q_EMIT windowActivated(windowId);
    hide();
}

void OverviewSurface::closeWindow(const QString &windowId)
{
    LINGMO_D(OverviewSurface);
    if (!d->windowManager)
        return;
    d->windowManager->closeWindow(windowId);
    Q_EMIT windowClosed(windowId);
}

void OverviewSurface::switchToWorkspace(int workspaceIndex)
{
    LINGMO_D(OverviewSurface);
    if (!d->windowManager)
        return;
    d->windowManager->switchToWorkspace(workspaceIndex);
}

void OverviewSurface::createWindow()
{
    LINGMO_D(OverviewSurface);
    Q_ASSERT(d->backend);

    d->window = d->backend->createLayerWindow(this,
                                               SurfaceLayer::Overlay,
                                               SurfaceEdge::None,
                                               0,
                                               d->screen);
    if (!d->window) {
        qCWarning(lcOverview) << "Backend returned null window for OverviewSurface";
        return;
    }

    d->window->setObjectName(QStringLiteral("LingmoOverviewWindow"));

    if (d->screen)
        d->window->setGeometry(d->screen->geometry());

    qCDebug(lcOverview) << "OverviewSurface window created";
}

} // namespace Lingmo
