#pragma once

#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <QDebug>
#include <QMetaType>
#include <cstdint>

namespace Lingmo {

Q_NAMESPACE_EXPORT(LINGMOSHELLSURFACES_EXPORT)

// Which physical screen edge the Panel is anchored to.
enum class SurfaceEdge : uint8_t {
    Top,
    Bottom,
    Left,
    Right,
    None
};
Q_ENUM_NS(SurfaceEdge)

// Wayland layer-shell layer the surface belongs to.
// Maps directly to zwlr_layer_shell_v1.layer values.
enum class SurfaceLayer : uint8_t {
    Background = 0, // below everything
    Bottom     = 1, // below windows but above background
    Top        = 2, // above windows (default for Panel)
    Overlay    = 3  // always on top (lockscreen, OSD)
};
Q_ENUM_NS(SurfaceLayer)

// Platform the surface is running on, detected at runtime.
enum class SurfacePlatform : uint8_t {
    Unknown,
    Wayland,
    X11,
    Offscreen // headless / tests
};
Q_ENUM_NS(SurfacePlatform)

// Describes the geometry request a surface sends to the compositor.
// On Wayland this maps to layer-surface configure events.
struct LINGMOSHELLSURFACES_EXPORT SurfaceGeometry {
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;

    [[nodiscard]] bool isValid() const noexcept { return width > 0 && height > 0; }

    bool operator==(const SurfaceGeometry &o) const noexcept
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }
    bool operator!=(const SurfaceGeometry &o) const noexcept { return !(*this == o); }
};

LINGMOSHELLSURFACES_EXPORT QDebug operator<<(QDebug dbg, SurfaceEdge edge);
LINGMOSHELLSURFACES_EXPORT QDebug operator<<(QDebug dbg, SurfaceLayer layer);
LINGMOSHELLSURFACES_EXPORT QDebug operator<<(QDebug dbg, SurfacePlatform platform);
LINGMOSHELLSURFACES_EXPORT QDebug operator<<(QDebug dbg, const SurfaceGeometry &g);

} // namespace Lingmo

Q_DECLARE_METATYPE(Lingmo::SurfaceEdge)
Q_DECLARE_METATYPE(Lingmo::SurfaceLayer)
Q_DECLARE_METATYPE(Lingmo::SurfacePlatform)
Q_DECLARE_METATYPE(Lingmo::SurfaceGeometry)
