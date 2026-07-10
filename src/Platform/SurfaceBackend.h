#pragma once

#include <LingmoShellSurfaces/Types.h>

class QScreen;
class QWindow;

namespace Lingmo {

class ShellSurface;

// SurfaceBackend — abstract interface for platform-specific surface operations.
//
// Each platform (Wayland, X11, Generic/offscreen) provides a concrete
// implementation.  ShellSurface and its subclasses call these methods instead
// of using platform-specific APIs directly.
//
// Backend objects are created via SurfaceBackend::create() which returns the
// best available backend for the running environment.
class SurfaceBackend
{
public:
    // Factory: returns WaylandSurfaceBackend, X11SurfaceBackend, or
    // GenericSurfaceBackend depending on QGuiApplication::platformName().
    static std::unique_ptr<SurfaceBackend> create();

    virtual ~SurfaceBackend() = default;

    [[nodiscard]] virtual SurfacePlatform platform() const = 0;

    // Creates and configures a native window for the given surface type.
    // Called once per show() cycle (may be called multiple times if
    // reload() is used).
    virtual QWindow *createLayerWindow(ShellSurface *surface,
                                       SurfaceLayer layer,
                                       SurfaceEdge edge,
                                       int exclusiveZone,
                                       QScreen *screen) = 0;

    // Updates the exclusive zone for an already-mapped window (e.g. when
    // the panel height changes at runtime).
    virtual void updateExclusiveZone(QWindow *window,
                                     SurfaceLayer layer,
                                     SurfaceEdge edge,
                                     int exclusiveZone) = 0;

    // Destroys the native surface resources.  The QWindow itself is
    // destroyed by the caller afterwards.
    virtual void destroyLayerWindow(QWindow *window) = 0;

protected:
    SurfaceBackend() = default;
};

} // namespace Lingmo
