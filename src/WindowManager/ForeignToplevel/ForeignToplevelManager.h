#pragma once
#include <QObject>
#include <QHash>

#include <QtWaylandClient/QWaylandClientExtensionTemplate>
#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"
#include "ForeignToplevelHandle.h"

namespace Lingmo {

// Binds zwlr_foreign_toplevel_manager_v1 from the Wayland registry and
// instantiates ForeignToplevelHandle objects for each open toplevel.
class ForeignToplevelManager
    : public QWaylandClientExtensionTemplate<ForeignToplevelManager>
    , public QtWayland::zwlr_foreign_toplevel_manager_v1
{
    Q_OBJECT
public:
    explicit ForeignToplevelManager(QObject *parent = nullptr);
    ~ForeignToplevelManager() override;

    // Returns all currently tracked handles (fully initialised).
    [[nodiscard]] QList<ForeignToplevelHandle *> handles() const;

Q_SIGNALS:
    void handleAdded(ForeignToplevelHandle *handle);
    void handleRemoved(const QString &handleId);
    void listStabilised();   // first time all initial handles are ready

private:
    // QtWayland protocol events
    void zwlr_foreign_toplevel_manager_v1_toplevel(
        struct ::zwlr_foreign_toplevel_handle_v1 *toplevel) override;
    void zwlr_foreign_toplevel_manager_v1_finished() override;

    QHash<QString, ForeignToplevelHandle *> handleMap;
    int pendingInitial = 0;
    bool stable = false;
};

} // namespace Lingmo
