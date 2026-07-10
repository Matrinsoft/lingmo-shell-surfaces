#include <LingmoShellSurfaces/Types.h>

#include <QDebug>

namespace Lingmo {

QDebug operator<<(QDebug dbg, SurfaceEdge edge)
{
    switch (edge) {
    case SurfaceEdge::Top:    dbg << "SurfaceEdge::Top";    break;
    case SurfaceEdge::Bottom: dbg << "SurfaceEdge::Bottom"; break;
    case SurfaceEdge::Left:   dbg << "SurfaceEdge::Left";   break;
    case SurfaceEdge::Right:  dbg << "SurfaceEdge::Right";  break;
    case SurfaceEdge::None:   dbg << "SurfaceEdge::None";   break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, SurfaceLayer layer)
{
    switch (layer) {
    case SurfaceLayer::Background: dbg << "SurfaceLayer::Background"; break;
    case SurfaceLayer::Bottom:     dbg << "SurfaceLayer::Bottom";     break;
    case SurfaceLayer::Top:        dbg << "SurfaceLayer::Top";        break;
    case SurfaceLayer::Overlay:    dbg << "SurfaceLayer::Overlay";    break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, SurfacePlatform platform)
{
    switch (platform) {
    case SurfacePlatform::Unknown:   dbg << "SurfacePlatform::Unknown";   break;
    case SurfacePlatform::Wayland:   dbg << "SurfacePlatform::Wayland";   break;
    case SurfacePlatform::X11:       dbg << "SurfacePlatform::X11";       break;
    case SurfacePlatform::Offscreen: dbg << "SurfacePlatform::Offscreen"; break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, const SurfaceGeometry &g)
{
    dbg.nospace() << "SurfaceGeometry(" << g.x << ", " << g.y
                  << ", " << g.width << "x" << g.height << ")";
    return dbg;
}

} // namespace Lingmo
