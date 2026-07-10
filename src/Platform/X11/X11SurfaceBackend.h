#pragma once

#include "../SurfaceBackend.h"

namespace Lingmo {

// X11 backend using override-redirect windows + EWMH hints.
//
// Panel:   Sets _NET_WM_WINDOW_TYPE_DOCK and _NET_WM_STRUT_PARTIAL so the
//          window manager reserves space and windows avoid the panel.
// Desktop: Sets _NET_WM_WINDOW_TYPE_DESKTOP so the WM stacks it below all
//          other windows.
// Overlay: Sets _NET_WM_WINDOW_TYPE_NOTIFICATION with override-redirect.
//
// All windows use Qt::FramelessWindowHint | Qt::Tool to suppress WM
// decoration and taskbar entries.
class X11SurfaceBackend : public SurfaceBackend
{
public:
    X11SurfaceBackend() = default;
    ~X11SurfaceBackend() override = default;

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

private:
    // Sets _NET_WM_WINDOW_TYPE atom on the native X11 window.
    void setWindowType(QWindow *window, SurfaceLayer layer) const;

    // Sets _NET_WM_STRUT_PARTIAL for panel windows.
    void setStrut(QWindow *window, SurfaceEdge edge, int thickness) const;
};

} // namespace Lingmo
