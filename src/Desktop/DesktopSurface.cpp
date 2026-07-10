#include <LingmoShellSurfaces/DesktopSurface.h>
#include "private/DesktopSurface_p.h"
#include "Platform/SurfaceBackend.h"

#include <QColor>
#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcDesktop, "lingmo.shell.surfaces.desktop")

namespace Lingmo {

// ── DesktopSurfacePrivate ──────────────────────────────────

DesktopSurfacePrivate::DesktopSurfacePrivate(DesktopSurface *q)
    : ShellSurfacePrivate(q)
{
}

// ── DesktopSurface ─────────────────────────────────────────

DesktopSurface::DesktopSurface(QObject *parent)
    : ShellSurface(new DesktopSurfacePrivate(this), parent)
{
}

DesktopSurface::~DesktopSurface() = default;

QString DesktopSurface::wallpaperSource() const
{
    LINGMO_CD(DesktopSurface);
    return d->wallpaperSource;
}

void DesktopSurface::setWallpaperSource(const QString &source)
{
    LINGMO_D(DesktopSurface);
    if (d->wallpaperSource == source)
        return;
    d->wallpaperSource = source;
    Q_EMIT wallpaperSourceChanged(source);
    qCDebug(lcDesktop) << "Wallpaper source changed to" << source;
}

QColor DesktopSurface::wallpaperColor() const
{
    LINGMO_CD(DesktopSurface);
    return d->wallpaperColor;
}

void DesktopSurface::setWallpaperColor(const QColor &color)
{
    LINGMO_D(DesktopSurface);
    if (d->wallpaperColor == color)
        return;
    d->wallpaperColor = color;
    Q_EMIT wallpaperColorChanged(color);
}

DesktopSurface::WallpaperFillMode DesktopSurface::fillMode() const
{
    LINGMO_CD(DesktopSurface);
    return d->fillMode;
}

void DesktopSurface::setFillMode(WallpaperFillMode mode)
{
    LINGMO_D(DesktopSurface);
    if (d->fillMode == mode)
        return;
    d->fillMode = mode;
    Q_EMIT fillModeChanged(mode);
}

SurfaceLayer DesktopSurface::layer() const
{
    return SurfaceLayer::Background;
}

void DesktopSurface::createWindow()
{
    LINGMO_D(DesktopSurface);
    Q_ASSERT(d->backend);

    d->window = d->backend->createLayerWindow(this,
                                               SurfaceLayer::Background,
                                               SurfaceEdge::None,
                                               0,
                                               d->screen);
    if (!d->window) {
        qCWarning(lcDesktop) << "Backend returned null window for DesktopSurface";
        return;
    }

    d->window->setObjectName(QStringLiteral("LingmoDesktopWindow"));

    // Desktop covers the full screen area.
    if (d->screen)
        d->window->setGeometry(d->screen->geometry());

    qCDebug(lcDesktop) << "DesktopSurface window created on screen"
                       << (d->screen ? d->screen->name() : QStringLiteral("(null)"));
}

} // namespace Lingmo
