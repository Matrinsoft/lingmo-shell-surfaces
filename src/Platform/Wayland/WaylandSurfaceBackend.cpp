#include "WaylandSurfaceBackend.h"
#include "LayerShellManager.h"
#include "LayerSurface.h"

#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QPointer>
#include <QScreen>
#include <QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>

Q_LOGGING_CATEGORY(lcWayland, "lingmo.shell.surfaces.wayland")

namespace Lingmo {

// ── Private ──────────────────────────────────────────────────────────────────
class WaylandSurfaceBackend::Private
{
public:
    LayerShellManager *shellManager = nullptr;
    QHash<QWindow *, LayerSurface *> layerSurfaces;
};

// ── Construction ─────────────────────────────────────────────────────────────
WaylandSurfaceBackend::WaylandSurfaceBackend()
    : d(std::make_unique<Private>())
{
    // LayerShellManager will auto-bind via the Wayland registry once
    // QGuiApplication's Wayland connection is ready.
    d->shellManager = new LayerShellManager();
    qCDebug(lcWayland) << "WaylandSurfaceBackend created";
}

WaylandSurfaceBackend::~WaylandSurfaceBackend()
{
    // Destroy any remaining layer surfaces before the manager goes away.
    const auto windows = d->layerSurfaces.keys();
    for (auto *w : windows)
        destroyLayerWindow(w);

    delete d->shellManager;
}

// ── SurfaceBackend interface ──────────────────────────────────────────────────
SurfacePlatform WaylandSurfaceBackend::platform() const
{
    return SurfacePlatform::Wayland;
}

bool WaylandSurfaceBackend::isLayerShellAvailable() const
{
    return d->shellManager && d->shellManager->isActive();
}

QWindow *WaylandSurfaceBackend::createLayerWindow(ShellSurface * /*surface*/,
                                                   SurfaceLayer   layer,
                                                   SurfaceEdge    edge,
                                                   int            exclusiveZone,
                                                   QScreen       *screen)
{
    auto *window = new QWindow();
    window->setFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::BypassWindowManagerHint);
    if (screen)
        window->setScreen(screen);

    // Set an initial geometry so the window has a valid size before the first
    // configure event arrives from the compositor.
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

    window->create(); // ensures wl_surface is allocated

    if (!d->shellManager || !d->shellManager->isActive()) {
        qCWarning(lcWayland) << "zwlr_layer_shell_v1 not available; "
                                "returning plain window without layer-shell role";
        d->layerSurfaces.insert(window, nullptr);
        return window;
    }

    auto *ni = QGuiApplication::platformNativeInterface();
    if (!ni) {
        qCWarning(lcWayland) << "No platform native interface; "
                                "returning plain window";
        d->layerSurfaces.insert(window, nullptr);
        return window;
    }

    auto *wlSurf = static_cast<struct wl_surface *>(
        ni->nativeResourceForWindow("surface", window));
    struct wl_output *wlOutput = screen
        ? static_cast<struct wl_output *>(ni->nativeResourceForScreen("output", screen))
        : nullptr;

    if (!wlSurf) {
        qCWarning(lcWayland) << "Could not obtain wl_surface from QWindow";
        d->layerSurfaces.insert(window, nullptr);
        return window;
    }

    const uint32_t layerVal = static_cast<uint32_t>(layer);
    auto *rawSurface = d->shellManager->get_layer_surface(wlSurf, wlOutput, layerVal,
                                                           "lingmo-shell");
    auto *ls = new LayerSurface(rawSurface, window);

    // Compute anchor bits for zwlr_layer_surface_v1.anchor
    // (top=1, bottom=2, left=4, right=8)
    uint32_t anchorBits = 0;
    switch (edge) {
    case SurfaceEdge::Top:    anchorBits = 1 | 4 | 8; break; // top | left | right  = 13
    case SurfaceEdge::Bottom: anchorBits = 2 | 4 | 8; break; // bottom | left | right = 14
    case SurfaceEdge::Left:   anchorBits = 4 | 1 | 2; break; // left | top | bottom = 7
    case SurfaceEdge::Right:  anchorBits = 8 | 1 | 2; break; // right | top | bottom = 11
    case SurfaceEdge::None:   anchorBits = 1 | 2 | 4 | 8; break; // all edges = 15
    }

    // Size hint: set 0 on the dimension the compositor should fill.
    uint32_t szW = 0, szH = 0;
    switch (edge) {
    case SurfaceEdge::Top:
    case SurfaceEdge::Bottom:
        szH = static_cast<uint32_t>(exclusiveZone);
        break;
    case SurfaceEdge::Left:
    case SurfaceEdge::Right:
        szW = static_cast<uint32_t>(exclusiveZone);
        break;
    default:
        break;
    }

    ls->setSize(szW, szH);
    ls->setAnchor(anchorBits);
    ls->setExclusiveZone(exclusiveZone);

    // Overlay layer surfaces (e.g. Overview) want on-demand keyboard focus;
    // panels and lower layers do not capture keyboard input.
    const uint32_t kbMode = (layer == SurfaceLayer::Overlay) ? 2u : 0u;
    ls->setKeyboardInteractivity(kbMode);

    // Re-fetch wlSurf inside the lambda since the pointer is valid as long as
    // the window is alive; use QPointer to avoid dangling window access.
    QPointer<QWindow> safeWindow = window;
    QObject::connect(ls, &LayerSurface::configured, window,
                     [safeWindow, ls](uint32_t serial, uint32_t w, uint32_t h) {
        if (!safeWindow)
            return;
        ls->ack_configure(serial);
        if (w > 0 && h > 0)
            safeWindow->resize(static_cast<int>(w), static_cast<int>(h));
        auto *ni2 = QGuiApplication::platformNativeInterface();
        if (ni2) {
            auto *wlS = static_cast<struct wl_surface *>(
                ni2->nativeResourceForWindow("surface", safeWindow));
            if (wlS)
                wl_surface_commit(wlS);
        }
    });

    QObject::connect(ls, &LayerSurface::closed, window,
                     [this, window]() {
        destroyLayerWindow(window);
    }, Qt::QueuedConnection);

    // Initial commit to announce the role to the compositor.
    wl_surface_commit(wlSurf);

    d->layerSurfaces.insert(window, ls);

    qCDebug(lcWayland) << "Created layer window:"
                       << "layer=" << static_cast<int>(layer)
                       << "edge=" << static_cast<int>(edge)
                       << "exclusiveZone=" << exclusiveZone
                       << "anchor=" << anchorBits;
    return window;
}

void WaylandSurfaceBackend::updateExclusiveZone(QWindow     *window,
                                                 SurfaceLayer /*layer*/,
                                                 SurfaceEdge  /*edge*/,
                                                 int          exclusiveZone)
{
    if (!window)
        return;

    LayerSurface *ls = d->layerSurfaces.value(window, nullptr);
    if (!ls)
        return;

    ls->setExclusiveZone(exclusiveZone);

    auto *ni = QGuiApplication::platformNativeInterface();
    if (ni) {
        auto *wlSurf = static_cast<struct wl_surface *>(
            ni->nativeResourceForWindow("surface", window));
        if (wlSurf)
            wl_surface_commit(wlSurf);
    }
}

void WaylandSurfaceBackend::destroyLayerWindow(QWindow *window)
{
    if (!window)
        return;

    LayerSurface *ls = d->layerSurfaces.take(window);
    delete ls;   // calls zwlr_layer_surface_v1::destroy() in destructor
    delete window;
}

} // namespace Lingmo
