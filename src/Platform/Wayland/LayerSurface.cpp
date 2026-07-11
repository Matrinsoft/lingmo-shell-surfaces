#include "LayerSurface.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcLayerSurface, "lingmo.shell.surfaces.layersurface")

namespace Lingmo {

LayerSurface::LayerSurface(struct ::zwlr_layer_surface_v1 *surface, QObject *parent)
    : QObject(parent)
    , QtWayland::zwlr_layer_surface_v1(surface)
{
    qCDebug(lcLayerSurface) << "LayerSurface created";
}

LayerSurface::~LayerSurface()
{
    if (QtWayland::zwlr_layer_surface_v1::object()) {
        QtWayland::zwlr_layer_surface_v1::destroy();
    }
    qCDebug(lcLayerSurface) << "LayerSurface destroyed";
}

void LayerSurface::setSize(uint32_t width, uint32_t height)
{
    set_size(width, height);
}

void LayerSurface::setAnchor(uint32_t anchor)
{
    set_anchor(anchor);
}

void LayerSurface::setExclusiveZone(int32_t zone)
{
    set_exclusive_zone(zone);
}

void LayerSurface::setMargin(int32_t top, int32_t right, int32_t bottom, int32_t left)
{
    set_margin(top, right, bottom, left);
}

void LayerSurface::setKeyboardInteractivity(uint32_t mode)
{
    set_keyboard_interactivity(mode);
}

void LayerSurface::setLayer(uint32_t layer)
{
    set_layer(layer);
}

void LayerSurface::zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height)
{
    qCDebug(lcLayerSurface) << "configure: serial=" << serial
                             << "size=" << width << "x" << height;
    Q_EMIT configured(serial, width, height);
}

void LayerSurface::zwlr_layer_surface_v1_closed()
{
    qCDebug(lcLayerSurface) << "closed";
    Q_EMIT closed();
}

} // namespace Lingmo
