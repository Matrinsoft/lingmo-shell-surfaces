#include <LingmoShellSurfaces/OverviewSurface.h>
#include "private/OverviewSurface_p.h"
#include "Platform/SurfaceBackend.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcOverview, "lingmo.shell.surfaces.overview")

namespace Lingmo {

// ── WindowListModel ────────────────────────────────────────

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WindowListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_windows.size();
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_windows.size())
        return {};

    const QVariantMap &w = m_windows.at(index.row());
    switch (role) {
    case WindowIdRole:       return w.value(QStringLiteral("windowId"));
    case TitleRole:          return w.value(QStringLiteral("title"));
    case ThumbnailUrlRole:   return w.value(QStringLiteral("thumbnailUrl"));
    case WorkspaceIndexRole: return w.value(QStringLiteral("workspaceIndex"));
    case IsMinimizedRole:    return w.value(QStringLiteral("isMinimized"));
    default:                 return {};
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const
{
    return {
        { WindowIdRole,       "windowId"       },
        { TitleRole,          "title"          },
        { ThumbnailUrlRole,   "thumbnailUrl"   },
        { WorkspaceIndexRole, "workspaceIndex" },
        { IsMinimizedRole,    "isMinimized"    },
    };
}

void WindowListModel::setWindows(const QList<QVariantMap> &windows)
{
    beginResetModel();
    m_windows = windows;
    endResetModel();
}

// ── WorkspaceListModel ─────────────────────────────────────

WorkspaceListModel::WorkspaceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WorkspaceListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_workspaces.size();
}

QVariant WorkspaceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_workspaces.size())
        return {};

    const QVariantMap &ws = m_workspaces.at(index.row());
    switch (role) {
    case WorkspaceIndexRole: return ws.value(QStringLiteral("workspaceIndex"));
    case NameRole:           return ws.value(QStringLiteral("name"));
    case WindowCountRole:    return ws.value(QStringLiteral("windowCount"));
    default:                 return {};
    }
}

QHash<int, QByteArray> WorkspaceListModel::roleNames() const
{
    return {
        { WorkspaceIndexRole, "workspaceIndex" },
        { NameRole,           "name"           },
        { WindowCountRole,    "windowCount"    },
    };
}

void WorkspaceListModel::setWorkspaces(const QList<QVariantMap> &workspaces)
{
    beginResetModel();
    m_workspaces = workspaces;
    endResetModel();
}

// ── OverviewSurfacePrivate ─────────────────────────────────

OverviewSurfacePrivate::OverviewSurfacePrivate(OverviewSurface *q)
    : ShellSurfacePrivate(q)
    , windowModel(new WindowListModel(q))
    , workspaceModel(new WorkspaceListModel(q))
{
}

void OverviewSurfacePrivate::connectToKWin()
{
    kwinInterface = new QDBusInterface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/KWin"),
        QStringLiteral("org.kde.KWin"),
        QDBusConnection::sessionBus(),
        q);

    if (!kwinInterface->isValid()) {
        qCWarning(lcOverview) << "KWin D-Bus interface not available; overview will be empty";
        return;
    }

    refreshWindowList();
    refreshWorkspaceList();

    ready = true;
    Q_EMIT static_cast<OverviewSurface *>(q)->readyChanged(true);
    qCInfo(lcOverview) << "Connected to KWin D-Bus interface";
}

void OverviewSurfacePrivate::refreshWindowList()
{
    if (!kwinInterface || !kwinInterface->isValid())
        return;

    // Placeholder — real integration uses org.kde.KWin.Effects or scripting bridge.
    windowModel->setWindows({});
}

void OverviewSurfacePrivate::refreshWorkspaceList()
{
    if (!kwinInterface || !kwinInterface->isValid())
        return;

    workspaceModel->setWorkspaces({});
}

// ── OverviewSurface ────────────────────────────────────────

OverviewSurface::OverviewSurface(QObject *parent)
    : ShellSurface(new OverviewSurfacePrivate(this), parent)
{
    LINGMO_D(OverviewSurface);
    QMetaObject::invokeMethod(this, [d] { d->connectToKWin(); },
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

SurfaceLayer OverviewSurface::layer() const
{
    return SurfaceLayer::Overlay;
}

void OverviewSurface::activateWindow(const QString &windowId)
{
    LINGMO_D(OverviewSurface);
    if (!d->kwinInterface || !d->kwinInterface->isValid())
        return;

    d->kwinInterface->call(QStringLiteral("activateWindow"), windowId);
    Q_EMIT windowActivated(windowId);
    hide();
}

void OverviewSurface::closeWindow(const QString &windowId)
{
    LINGMO_D(OverviewSurface);
    if (!d->kwinInterface || !d->kwinInterface->isValid())
        return;

    d->kwinInterface->call(QStringLiteral("closeWindow"), windowId);
    Q_EMIT windowClosed(windowId);
}

void OverviewSurface::switchToWorkspace(int workspaceIndex)
{
    LINGMO_D(OverviewSurface);
    if (!d->kwinInterface || !d->kwinInterface->isValid())
        return;

    d->kwinInterface->call(QStringLiteral("setCurrentDesktop"), workspaceIndex + 1);
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
