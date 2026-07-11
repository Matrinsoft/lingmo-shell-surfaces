#pragma once

#include <LingmoShellSurfaces/OverviewSurface.h>
#include "ShellSurface_p.h"

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

namespace Lingmo {

class WindowManagerBackend;

// ── Window model ─────────────────────────────────────────────────────────────
class WindowListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        WindowIdRole     = Qt::UserRole + 1,
        TitleRole,
        AppIdRole,
        ThumbnailUrlRole,
        WorkspaceIndexRole,
        IsMinimizedRole,
        IsActiveRole,
    };

    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWindows(const QList<QVariantMap> &windows);

private:
    QList<QVariantMap> m_windows;
};

// ── Workspace model ───────────────────────────────────────────────────────────
class WorkspaceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        WorkspaceIndexRole = Qt::UserRole + 1,
        NameRole,
        WindowCountRole,
        ActiveRole,         // bool — true when this is the current workspace
    };

    explicit WorkspaceListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWorkspaces(const QList<QVariantMap> &workspaces);

    // Updates the current workspace index and emits dataChanged for the
    // affected rows so QML delegates refresh their 'active' role.
    void setCurrentWorkspaceIndex(int idx);

    int currentWorkspaceIndex = 0;

private:
    QList<QVariantMap> m_workspaces;
};

// ── OverviewSurfacePrivate ────────────────────────────────────────────────────
class OverviewSurfacePrivate : public ShellSurfacePrivate
{
public:
    explicit OverviewSurfacePrivate(OverviewSurface *q);

    bool ready = false;
    WindowListModel    *windowModel    = nullptr;
    WorkspaceListModel *workspaceModel = nullptr;

    WindowManagerBackend *windowManager = nullptr;

    void connectToWindowManager();
};

} // namespace Lingmo
