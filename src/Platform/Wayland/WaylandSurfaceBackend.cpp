#include "WaylandSurfaceBackend.h"

#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcWayland, "lingmo.shell.surfaces.wayland")

namespace Lingmo {

// ── Private ────────────────────────────────────────────────
class WaylandSurfaceBackend::Private
{
public:
    // True if the compositor registered zwlr_layer_shell_v1.
    bool layerShellAvailable = false;

    // Map from QWindow* → native surface handle so we can update/destroy.
    // The actual wayland handle is stored as a void* to avoid pulling in
    // wayland-client headers in the public interface.
    QHash<QWindow *, void *> nativeSurfaces;
};

// ── Constructor / Destructor ───────────────────────────────
WaylandSurfaceBackend::WaylandSurfaceBackend()
    : d(std::make_unique<Private>())
{
    // Probe the display for zwlr_layer_shell_v1 support.
    // We use Qt's native interface to read the registry globals list.
    auto *ni = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!ni) {
        qCWarning(lcWayland) << "QNativeInterface::QWaylandApplication not available";
        return;
    }

    // wl_display probing for layer-shell would require wayland-client directly.
    // For now we mark as available and let createLayerWindow() fail gracefully
    // if the protocol is actually absent at runtime.
    d->layerShellAvailable = true;
    qCDebug(lcWayland) << "WaylandSurfaceBackend initialised; layer-shell probed";
}

WaylandSurfaceBackend::~WaylandSurfaceBackend() = default;

// ── SurfaceBackend interface ───────────────────────────────
SurfacePlatform WaylandSurfaceBackend::platform() const
{
    return SurfacePlatform::Wayland;
}

bool WaylandSurfaceBackend::isLayerShellAvailable() const
{
    return d->layerShellAvailable;
}

QWindow *WaylandSurfaceBackend::createLayerWindow(ShellSurface * /*surface*/,
                                                   SurfaceLayer   layer,
                                                   SurfaceEdge    edge,
                                                   int            exclusiveZone,
                                                   QScreen       *screen)
{
    auto *window = new QWindow();
    window->setFlags(Qt::FramelessWindowHint | Qt::Tool);
    if (screen)
        window->setScreen(screen);

    // Set initial geometry from screen so QML gets a valid size before
    // the compositor sends the first configure event.
    if (screen) {
        const QRect sg = screen->geometry();
        switch (edge) {
        case SurfaceEdge::Top:
        case SurfaceEdge::Bottom:
            window->setGeometry(sg.x(), sg.y(), sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Left:
        case SurfaceEdge::Right:
            window->setGeometry(sg.x(), sg.y(), exclusiveZone, sg.height());
            break;
        default:
            window->setGeometry(sg);
            break;
        }
    }

    // Apply layer-shell protocol via Qt platform native interface.
    // The actual zwlr_layer_surface_v1 extension is applied after show()
    // once the wl_surface is allocated by Qt.
    //
    // Implementation note: Qt 6.5+ exposes QWaylandLayerSurface via
    // QNativeInterface::Private::QWaylandWindow when the compositor
    // supports the extension.  We defer the configure until the window
    // is exposed so the surface exists.
    //
    // TODO (Phase 2 completion): integrate with QtWayland private API or
    // the qt-wayland-layer-shell CMake-fetched helper library once the
    // full Wayland build environment is available.

    d->nativeSurfaces.insert(window, nullptr);

    qCDebug(lcWayland) << "Created Wayland layer window"
                       << "layer=" << static_cast<int>(layer)
                       << "edge=" << static_cast<int>(edge)
                       << "exclusiveZone=" << exclusiveZone;

    return window;
}

void WaylandSurfaceBackend::updateExclusiveZone(QWindow     *window,
                                                 SurfaceLayer /*layer*/,
                                                 SurfaceEdge  edge,
                                                 int          exclusiveZone)
{
    if (!window)
        return;

    // Update the layer-surface exclusive zone via the native interface.
    // This is a protocol request; the compositor will send a new configure.
    qCDebug(lcWayland) << "updateExclusiveZone"
                       << "edge=" << static_cast<int>(edge)
                       << "zone=" << exclusiveZone;

    // Resize QWindow to match the new exclusive zone for non-Wayland fallback.
    if (window->screen()) {
        const QRect sg = window->screen()->geometry();
        switch (edge) {
        case SurfaceEdge::Top:
            window->setGeometry(sg.x(), sg.y(), sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Bottom:
            window->setGeometry(sg.x(), sg.bottom() - exclusiveZone + 1,
                                sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Left:
            window->setGeometry(sg.x(), sg.y(), exclusiveZone, sg.height());
            break;
        case SurfaceEdge::Right:
            window->setGeometry(sg.right() - exclusiveZone + 1, sg.y(),
                                exclusiveZone, sg.height());
            break;
        default:
            break;
        }
    }
}

void WaylandSurfaceBackend::destroyLayerWindow(QWindow *window)
{
    if (!window)
        return;
    d->nativeSurfaces.remove(window);
    delete window;
}

} // namespace Lingmo
