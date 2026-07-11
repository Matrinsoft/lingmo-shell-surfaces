#pragma once
#include <QObject>
#include <cstdint>
#include "qwayland-wlr-layer-shell-unstable-v1.h"

namespace Lingmo {

// Wraps a zwlr_layer_surface_v1. Takes ownership of the surface.
class LayerSurface : public QObject, public QtWayland::zwlr_layer_surface_v1
{
    Q_OBJECT
public:
    explicit LayerSurface(struct ::zwlr_layer_surface_v1 *surface, QObject *parent = nullptr);
    ~LayerSurface() override;

    void setSize(uint32_t width, uint32_t height);
    void setAnchor(uint32_t anchor);
    void setExclusiveZone(int32_t zone);
    void setMargin(int32_t top, int32_t right, int32_t bottom, int32_t left);
    void setKeyboardInteractivity(uint32_t mode);
    void setLayer(uint32_t layer);

Q_SIGNALS:
    void configured(uint32_t serial, uint32_t width, uint32_t height);
    void closed();

protected:
    void zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height) override;
    void zwlr_layer_surface_v1_closed() override;
};

} // namespace Lingmo
