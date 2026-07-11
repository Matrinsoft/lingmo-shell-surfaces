#pragma once

#include "../SurfaceBackend.h"

namespace Lingmo {

// Generic (offscreen / unknown platform) backend.
// Creates a plain QWindow with no platform-specific extensions.
// Used in tests and as a safe fallback.
class GenericSurfaceBackend : public SurfaceBackend
{
public:
    GenericSurfaceBackend() = default;
    ~GenericSurfaceBackend() override = default;

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
};

} // namespace Lingmo
