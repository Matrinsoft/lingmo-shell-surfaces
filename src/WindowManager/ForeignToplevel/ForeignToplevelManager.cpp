#include "ForeignToplevelManager.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcForeignMgr, "lingmo.shell.surfaces.foreigntoplevel.manager")

namespace Lingmo {

ForeignToplevelManager::ForeignToplevelManager(QObject *parent)
    : QWaylandClientExtensionTemplate<ForeignToplevelManager>(3)
    , QObject(parent)
{
    // QWaylandClientExtensionTemplate automatically binds the global once
    // the Wayland registry emits the global event for our interface.
    connect(this, &QWaylandClientExtension::activeChanged, this,
            [this](bool active) {
                qCInfo(lcForeignMgr) << "zwlr_foreign_toplevel_manager_v1 active:" << active;
            });
}

ForeignToplevelManager::~ForeignToplevelManager()
{
    qDeleteAll(handleMap);
    handleMap.clear();
    if (isInitialized())
        stop();
}

QList<ForeignToplevelHandle *> ForeignToplevelManager::handles() const
{
    return handleMap.values();
}

// ── Protocol events ───────────────────────────────────────────────────────────

void ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_toplevel(
    struct ::zwlr_foreign_toplevel_handle_v1 *toplevel)
{
    auto *handle = new ForeignToplevelHandle(toplevel, this);

    ++pendingInitial;

    connect(handle, &ForeignToplevelHandle::ready, this, [this, handle]() {
        handleMap.insert(handle->handleId(), handle);
        Q_EMIT handleAdded(handle);

        // Decrement the pending counter to know when the initial burst is done.
        if (!stable) {
            --pendingInitial;
            if (pendingInitial <= 0) {
                stable = true;
                Q_EMIT listStabilised();
            }
        }

        // Re-emit handleAdded → callers rebuild their window list on changed().
        connect(handle, &ForeignToplevelHandle::changed, this,
                [this, handle]() { Q_EMIT handleAdded(handle); });

        connect(handle, &ForeignToplevelHandle::closed, this,
                [this, handle]() {
                    const QString id = handle->handleId();
                    handleMap.remove(id);
                    Q_EMIT handleRemoved(id);
                    handle->deleteLater();
                });
    });

    qCDebug(lcForeignMgr) << "New toplevel handle received (pending init)";
}

void ForeignToplevelManager::zwlr_foreign_toplevel_manager_v1_finished()
{
    qCInfo(lcForeignMgr) << "zwlr_foreign_toplevel_manager_v1 finished by compositor";
}

} // namespace Lingmo
