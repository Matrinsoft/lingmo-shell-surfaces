#include "SurfaceBackend.h"
#include "Generic/GenericSurfaceBackend.h"

#if LINGMO_HAVE_WAYLAND
#include "Wayland/WaylandSurfaceBackend.h"
#endif

#if LINGMO_HAVE_X11
#include "X11/X11SurfaceBackend.h"
#endif

#include <QGuiApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcSurfaceBackend, "lingmo.shell.surfaces.backend")

namespace Lingmo {

std::unique_ptr<SurfaceBackend> SurfaceBackend::create()
{
    const QString platformName = QGuiApplication::platformName();
    qCDebug(lcSurfaceBackend) << "Detected Qt platform:" << platformName;

#if LINGMO_HAVE_WAYLAND
    if (platformName == QStringLiteral("wayland") ||
        platformName.startsWith(QStringLiteral("wayland-")))
    {
        auto backend = std::make_unique<WaylandSurfaceBackend>();
        if (backend->isLayerShellAvailable()) {
            qCInfo(lcSurfaceBackend) << "Using Wayland layer-shell backend";
            return backend;
        }
        qCWarning(lcSurfaceBackend)
            << "Wayland platform detected but zwlr_layer_shell_v1 not available;"
               " falling back to generic backend";
    }
#endif

#if LINGMO_HAVE_X11
    if (platformName == QStringLiteral("xcb")) {
        qCInfo(lcSurfaceBackend) << "Using X11 (xcb) backend";
        return std::make_unique<X11SurfaceBackend>();
    }
#endif

    qCInfo(lcSurfaceBackend) << "Using generic (offscreen) backend";
    return std::make_unique<GenericSurfaceBackend>();
}

} // namespace Lingmo
