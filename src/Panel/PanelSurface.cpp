#include <LingmoShellSurfaces/PanelSurface.h>
#include "private/PanelSurface_p.h"
#include "Platform/SurfaceBackend.h"

#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcPanel, "lingmo.shell.surfaces.panel")

namespace Lingmo {

// ── PanelSurfacePrivate ────────────────────────────────────

PanelSurfacePrivate::PanelSurfacePrivate(PanelSurface *q)
    : ShellSurfacePrivate(q)
{
}

void PanelSurfacePrivate::updateExclusiveZone()
{
    exclusiveZone = autohide ? 0 : height;
    auto *pq = static_cast<PanelSurface *>(q);
    Q_EMIT pq->exclusiveZoneChanged(exclusiveZone);

    if (window && backend)
        backend->updateExclusiveZone(window, SurfaceLayer::Top, edge, exclusiveZone);
}

void PanelSurfacePrivate::applyEdgeToWindow()
{
    if (!window || !screen)
        return;

    const QRect sg = screen->geometry();
    switch (edge) {
    case SurfaceEdge::Top:
        window->setGeometry(sg.x(), sg.y(), sg.width(), height);
        break;
    case SurfaceEdge::Bottom:
        window->setGeometry(sg.x(), sg.bottom() - height + 1, sg.width(), height);
        break;
    case SurfaceEdge::Left:
        window->setGeometry(sg.x(), sg.y(), height, sg.height());
        break;
    case SurfaceEdge::Right:
        window->setGeometry(sg.right() - height + 1, sg.y(), height, sg.height());
        break;
    default:
        break;
    }
}

// ── PanelSurface ───────────────────────────────────────────

PanelSurface::PanelSurface(QObject *parent)
    : ShellSurface(new PanelSurfacePrivate(this), parent)
{
}

PanelSurface::~PanelSurface() = default;

SurfaceEdge PanelSurface::edge() const
{
    LINGMO_CD(PanelSurface);
    return d->edge;
}

void PanelSurface::setEdge(SurfaceEdge edge)
{
    LINGMO_D(PanelSurface);
    if (d->edge == edge)
        return;
    d->edge = edge;
    Q_EMIT edgeChanged(edge);

    if (d->visible)
        reload();
    else
        d->applyEdgeToWindow();
}

int PanelSurface::panelHeight() const
{
    LINGMO_CD(PanelSurface);
    return d->height;
}

void PanelSurface::setPanelHeight(int height)
{
    LINGMO_D(PanelSurface);
    if (d->height == height || height <= 0)
        return;
    d->height = height;
    Q_EMIT panelHeightChanged(height);
    d->updateExclusiveZone();
    d->applyEdgeToWindow();
}

bool PanelSurface::autohide() const
{
    LINGMO_CD(PanelSurface);
    return d->autohide;
}

void PanelSurface::setAutohide(bool autohide)
{
    LINGMO_D(PanelSurface);
    if (d->autohide == autohide)
        return;
    d->autohide = autohide;
    Q_EMIT autohideChanged(autohide);
    d->updateExclusiveZone();
}

int PanelSurface::exclusiveZone() const
{
    LINGMO_CD(PanelSurface);
    return d->exclusiveZone;
}

void PanelSurface::appendApplet(QObject *applet)
{
    LINGMO_D(PanelSurface);
    if (!applet)
        return;
    applet->setParent(this);
    d->applets.append(applet);
    qCDebug(lcPanel) << "Applet appended; total=" << d->applets.size();
}

void PanelSurface::removeApplet(QObject *applet)
{
    LINGMO_D(PanelSurface);
    for (int i = 0; i < d->applets.size(); ++i) {
        if (d->applets.at(i) == applet) {
            delete d->applets.takeAt(i);
            return;
        }
    }
}

void PanelSurface::removeApplet(int index)
{
    LINGMO_D(PanelSurface);
    if (index < 0 || index >= d->applets.size())
        return;
    delete d->applets.takeAt(index);
}

int PanelSurface::appletCount() const
{
    LINGMO_CD(PanelSurface);
    return d->applets.size();
}

SurfaceLayer PanelSurface::layer() const
{
    return SurfaceLayer::Top;
}

void PanelSurface::createWindow()
{
    LINGMO_D(PanelSurface);
    Q_ASSERT(d->backend);

    d->exclusiveZone = d->autohide ? 0 : d->height;
    d->window = d->backend->createLayerWindow(this,
                                               SurfaceLayer::Top,
                                               d->edge,
                                               d->exclusiveZone,
                                               d->screen);
    if (!d->window) {
        qCWarning(lcPanel) << "Backend returned null window for PanelSurface";
        return;
    }

    d->window->setObjectName(QStringLiteral("LingmoPanelWindow"));
    d->applyEdgeToWindow();
    qCDebug(lcPanel) << "PanelSurface window created"
                     << "edge=" << d->edge
                     << "height=" << d->height;
}

} // namespace Lingmo
