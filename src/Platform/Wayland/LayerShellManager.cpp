#include "LayerShellManager.h"

#include <QLoggingCategory>
#include <algorithm>

Q_LOGGING_CATEGORY(lcLayerShell, "lingmo.shell.surfaces.layershell")

namespace Lingmo {

LayerShellManager::LayerShellManager(QObject *parent)
    : QWaylandClientExtensionTemplate<LayerShellManager>(/* version */ 4)
    , QtWayland::zwlr_layer_shell_v1()
{
    setParent(parent);
    // QWaylandClientExtensionTemplate registers itself on construction and
    // calls init() once the compositor advertises the matching interface.
    qCDebug(lcLayerShell) << "LayerShellManager created, waiting for compositor binding";
}

LayerShellManager::~LayerShellManager()
{
    if (isActive()) {
        // version >= 3 added a destructor request; guard against older servers
        // by checking the bound version.
        if (QtWayland::zwlr_layer_shell_v1::object()) {
            QtWayland::zwlr_layer_shell_v1::destroy();
        }
    }
}

void LayerShellManager::init(struct ::wl_registry *registry, int id, int version)
{
    // Request at most the version we were compiled for (4).
    const int boundVersion = std::min(version, 4);
    QtWayland::zwlr_layer_shell_v1::init(registry, id, boundVersion);
    qCInfo(lcLayerShell) << "zwlr_layer_shell_v1 bound, version" << boundVersion;
}

} // namespace Lingmo
