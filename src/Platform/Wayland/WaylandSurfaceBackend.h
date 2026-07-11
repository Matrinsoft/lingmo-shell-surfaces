#pragma once
#include "../SurfaceBackend.h"
#include <QHash>
#include <memory>

class QWindow;
class QScreen;

namespace Lingmo {

class LayerShellManager;
class LayerSurface;

class WaylandSurfaceBackend : public SurfaceBackend
{
public:
    WaylandSurfaceBackend();
    ~WaylandSurfaceBackend() override;

    SurfacePlatform platform() const override;
    [[nodiscard]] bool isLayerShellAvailable() const;

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
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Lingmo
