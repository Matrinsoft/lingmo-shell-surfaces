#include "X11SurfaceBackend.h"

#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

#if LINGMO_HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xcb/xcb.h>
#endif

Q_LOGGING_CATEGORY(lcX11, "lingmo.shell.surfaces.x11")

namespace Lingmo {

SurfacePlatform X11SurfaceBackend::platform() const
{
    return SurfacePlatform::X11;
}

QWindow *X11SurfaceBackend::createLayerWindow(ShellSurface * /*surface*/,
                                               SurfaceLayer   layer,
                                               SurfaceEdge    edge,
                                               int            exclusiveZone,
                                               QScreen       *screen)
{
    auto *window = new QWindow();
    // Override-redirect: the window manager will not manage this window.
    window->setFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::BypassWindowManagerHint);

    if (screen) {
        window->setScreen(screen);
        const QRect sg = screen->geometry();
        switch (edge) {
        case SurfaceEdge::Top:
            window->setGeometry(sg.x(), sg.y(), sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Bottom:
            window->setGeometry(sg.x(), sg.bottom() - exclusiveZone + 1,
                                sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Left:
            window->setGeometry(sg.x(), sg.y(), exclusiveZone, sg.height());
            break;
        case SurfaceEdge::Right:
            window->setGeometry(sg.right() - exclusiveZone + 1, sg.y(),
                                exclusiveZone, sg.height());
            break;
        default:
            window->setGeometry(sg);
            break;
        }
    }

    // We need the window to be shown (mapped) before we can set X11 atoms.
    // Connect to the expose event so we can apply EWMH hints post-map.
    QObject::connect(window, &QWindow::visibleChanged, window,
                     [this, window, layer, edge, exclusiveZone](bool visible) {
                         if (visible) {
                             setWindowType(window, layer);
                             if (layer == SurfaceLayer::Top)
                                 setStrut(window, edge, exclusiveZone);
                         }
                     });

    qCDebug(lcX11) << "Created X11 shell window"
                   << "layer=" << static_cast<int>(layer)
                   << "edge=" << static_cast<int>(edge);

    return window;
}

void X11SurfaceBackend::updateExclusiveZone(QWindow    *window,
                                             SurfaceLayer layer,
                                             SurfaceEdge  edge,
                                             int          exclusiveZone)
{
    if (!window)
        return;

    if (window->screen()) {
        const QRect sg = window->screen()->geometry();
        switch (edge) {
        case SurfaceEdge::Top:
            window->setGeometry(sg.x(), sg.y(), sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Bottom:
            window->setGeometry(sg.x(), sg.bottom() - exclusiveZone + 1,
                                sg.width(), exclusiveZone);
            break;
        case SurfaceEdge::Left:
            window->setGeometry(sg.x(), sg.y(), exclusiveZone, sg.height());
            break;
        case SurfaceEdge::Right:
            window->setGeometry(sg.right() - exclusiveZone + 1, sg.y(),
                                exclusiveZone, sg.height());
            break;
        default:
            break;
        }
    }

    if (window->isVisible() && layer == SurfaceLayer::Top)
        setStrut(window, edge, exclusiveZone);
}

void X11SurfaceBackend::destroyLayerWindow(QWindow *window)
{
    delete window;
}

void X11SurfaceBackend::setWindowType(QWindow *window, SurfaceLayer layer) const
{
#if LINGMO_HAVE_X11
    if (!window || !window->winId())
        return;

    auto *display = reinterpret_cast<Display *>(
        qGuiApp->nativeInterface<QNativeInterface::QX11Application>()
            ? qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display()
            : nullptr);
    if (!display)
        return;

    Atom netWmWindowType     = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom netWmWindowTypeDock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom netWmWindowTypeDesktop = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    Atom netWmWindowTypeNotification = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);

    Atom typeAtom = netWmWindowTypeDock; // default for panel
    switch (layer) {
    case SurfaceLayer::Background:
        typeAtom = netWmWindowTypeDesktop;
        break;
    case SurfaceLayer::Overlay:
        typeAtom = netWmWindowTypeNotification;
        break;
    default:
        typeAtom = netWmWindowTypeDock;
        break;
    }

    XChangeProperty(display, static_cast<Window>(window->winId()),
                    netWmWindowType, XA_ATOM, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(&typeAtom), 1);
    XFlush(display);
#else
    Q_UNUSED(window)
    Q_UNUSED(layer)
#endif
}

void X11SurfaceBackend::setStrut(QWindow    *window,
                                  SurfaceEdge  edge,
                                  int          thickness) const
{
#if LINGMO_HAVE_X11
    if (!window || !window->winId() || !window->screen())
        return;

    auto *display = reinterpret_cast<Display *>(
        qGuiApp->nativeInterface<QNativeInterface::QX11Application>()
            ? qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display()
            : nullptr);
    if (!display)
        return;

    const QRect sg = window->screen()->geometry();
    // _NET_WM_STRUT_PARTIAL: left, right, top, bottom,
    //   left_start_y, left_end_y, right_start_y, right_end_y,
    //   top_start_x, top_end_x, bottom_start_x, bottom_end_x
    long strut[12] = {};
    switch (edge) {
    case SurfaceEdge::Left:
        strut[0] = sg.x() + thickness;
        strut[4] = sg.y();
        strut[5] = sg.bottom();
        break;
    case SurfaceEdge::Right:
        strut[1] = sg.width() - sg.right() - 1 + thickness;
        strut[6] = sg.y();
        strut[7] = sg.bottom();
        break;
    case SurfaceEdge::Top:
        strut[2] = sg.y() + thickness;
        strut[8] = sg.x();
        strut[9] = sg.right();
        break;
    case SurfaceEdge::Bottom:
        strut[3] = sg.height() - sg.bottom() - 1 + thickness;
        strut[10] = sg.x();
        strut[11] = sg.right();
        break;
    default:
        return;
    }

    Atom netWmStrutPartial = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
    XChangeProperty(display, static_cast<Window>(window->winId()),
                    netWmStrutPartial, XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(strut), 12);
    XFlush(display);
#else
    Q_UNUSED(window)
    Q_UNUSED(edge)
    Q_UNUSED(thickness)
#endif
}

} // namespace Lingmo
