#pragma once

#include <LingmoShellSurfaces/OverviewSurface.h>
#include "ShellSurface_p.h"

#include <QAbstractListModel>
#include <QDBusInterface>
#include <QList>
#include <QVariantMap>

namespace Lingmo {

// ── Window model ────────────────────────────────────────────
class WindowListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        WindowIdRole = Qt::UserRole + 1,
        TitleRole,
        ThumbnailUrlRole,
        WorkspaceIndexRole,
        IsMinimizedRole,
    };

    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWindows(const QList<QVariantMap> &windows);

private:
    QList<QVariantMap> m_windows;
};

// ── Workspace model ─────────────────────────────────────────
class WorkspaceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        WorkspaceIndexRole = Qt::UserRole + 1,
        NameRole,
        WindowCountRole,
    };

    explicit WorkspaceListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWorkspaces(const QList<QVariantMap> &workspaces);

private:
    QList<QVariantMap> m_workspaces;
};

// ── OverviewSurfacePrivate ───────────────────────────────────
class OverviewSurfacePrivate : public ShellSurfacePrivate
{
public:
    explicit OverviewSurfacePrivate(OverviewSurface *q);

    bool ready = false;
    WindowListModel *windowModel = nullptr;
    WorkspaceListModel *workspaceModel = nullptr;

    QDBusInterface *kwinInterface = nullptr;

    void connectToKWin();
    void refreshWindowList();
    void refreshWorkspaceList();
};

} // namespace Lingmo
