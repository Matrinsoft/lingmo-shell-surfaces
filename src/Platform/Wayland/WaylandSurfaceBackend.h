#pragma once

#include "../SurfaceBackend.h"

namespace Lingmo {

// Wayland backend using wlr-layer-shell-unstable-v1.
//
// This backend requests zwlr_layer_shell_v1 from the Wayland registry and
// creates a zwlr_layer_surface_v1 for each shell surface.  Qt's
// QPlatformNativeInterface is used to obtain the native wl_surface from a
// QWindow so the layer-shell extension can be applied.
//
// Protocol requirement: the compositor must support wlr-layer-shell-unstable-v1
// (KWin >= 5.27 / wlroots-based compositors).
//
// Configure flow:
//   createLayerWindow() → creates QWindow → gets wl_surface via NativeInterface
//   → binds zwlr_layer_surface_v1 → sets layer/anchor/exclusive zone
//   → QWindow::show() triggers commit → compositor sends configure event
//   → configure ACK sent back
class WaylandSurfaceBackend : public SurfaceBackend
{
public:
    WaylandSurfaceBackend();
    ~WaylandSurfaceBackend() override;

    SurfacePlatform platform() const override;

    QWindow *createLayerWindow(ShellSurface *surface,
                               SurfaceLayer layer,
                               SurfaceEdge edge,
                               int exclusiveZone,
                               QScreen *screen) override;

    void updateExclusiveZone(QWindow *window,
                             SurfaceLayer layer,
                             SurfaceEdge edge,
                             int exclusiveZone) override;

    void destroyLayerWindow(QWindow *window) override;

    // Returns true if the running compositor advertised zwlr_layer_shell_v1.
    [[nodiscard]] bool isLayerShellAvailable() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Lingmo
