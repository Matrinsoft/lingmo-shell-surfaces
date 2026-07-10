#include "Generic/GenericSurfaceBackend.h"

#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

namespace Lingmo {

SurfacePlatform GenericSurfaceBackend::platform() const
{
    return SurfacePlatform::Offscreen;
}

QWindow *GenericSurfaceBackend::createLayerWindow(ShellSurface * /*surface*/,
                                                   SurfaceLayer   /*layer*/,
                                                   SurfaceEdge    edge,
                                                   int            exclusiveZone,
                                                   QScreen       *screen)
{
    Q_UNUSED(exclusiveZone)

    auto *window = new QWindow();
    window->setFlags(Qt::FramelessWindowHint | Qt::Tool);

    if (screen) {
        window->setScreen(screen);
        const QRect screenGeo = screen->geometry();

        // Position the window according to the edge (best-effort on generic).
        switch (edge) {
        case SurfaceEdge::Top:
            window->setGeometry(screenGeo.x(), screenGeo.y(),
                                screenGeo.width(), exclusiveZone);
            break;
        case SurfaceEdge::Bottom:
            window->setGeometry(screenGeo.x(),
                                screenGeo.bottom() - exclusiveZone + 1,
                                screenGeo.width(), exclusiveZone);
            break;
        case SurfaceEdge::Left:
            window->setGeometry(screenGeo.x(), screenGeo.y(),
                                exclusiveZone, screenGeo.height());
            break;
        case SurfaceEdge::Right:
            window->setGeometry(screenGeo.right() - exclusiveZone + 1,
                                screenGeo.y(),
                                exclusiveZone, screenGeo.height());
            break;
        default:
            window->setGeometry(screenGeo);
            break;
        }
    }

    return window;
}

void GenericSurfaceBackend::updateExclusiveZone(QWindow     *window,
                                                 SurfaceLayer /*layer*/,
                                                 SurfaceEdge  edge,
                                                 int          exclusiveZone)
{
    if (!window || !window->screen())
        return;

    const QRect screenGeo = window->screen()->geometry();
    switch (edge) {
    case SurfaceEdge::Top:
        window->setHeight(exclusiveZone);
        break;
    case SurfaceEdge::Bottom:
        window->setY(screenGeo.bottom() - exclusiveZone + 1);
        window->setHeight(exclusiveZone);
        break;
    case SurfaceEdge::Left:
        window->setWidth(exclusiveZone);
        break;
    case SurfaceEdge::Right:
        window->setX(screenGeo.right() - exclusiveZone + 1);
        window->setWidth(exclusiveZone);
        break;
    default:
        break;
    }
}

void GenericSurfaceBackend::destroyLayerWindow(QWindow *window)
{
    delete window;
}

} // namespace Lingmo
