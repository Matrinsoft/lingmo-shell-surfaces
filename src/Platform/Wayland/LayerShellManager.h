#pragma once
#include <QtWaylandClient/QWaylandClientExtension>
#include "qwayland-wlr-layer-shell-unstable-v1.h"
#include <QObject>

namespace Lingmo {

// Binds to zwlr_layer_shell_v1 in the Wayland registry.
class LayerShellManager : public QWaylandClientExtensionTemplate<LayerShellManager>,
                          public QtWayland::zwlr_layer_shell_v1
{
    Q_OBJECT
public:
    explicit LayerShellManager(QObject *parent = nullptr);
    ~LayerShellManager() override;

    void init(struct ::wl_registry *registry, int id, int version);
};

} // namespace Lingmo
