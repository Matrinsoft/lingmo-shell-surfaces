#include <LingmoShellSurfaces/ShellSurface.h>
#include "private/ShellSurface_p.h"
#include "Platform/SurfaceBackend.h"

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>
#include <QWindow>

Q_LOGGING_CATEGORY(lcShellSurface, "lingmo.shell.surfaces.surface")

namespace Lingmo {

// ── ShellSurfacePrivate ────────────────────────────────────

ShellSurfacePrivate::ShellSurfacePrivate(ShellSurface *q)
    : q(q)
    , screen(QGuiApplication::primaryScreen())
    , platform(detectPlatform())
{
}

ShellSurfacePrivate::~ShellSurfacePrivate()
{
    delete window;
    window = nullptr;
}

SurfacePlatform ShellSurfacePrivate::detectPlatform()
{
    const QString name = QGuiApplication::platformName();
    if (name == QStringLiteral("wayland") || name.startsWith(QStringLiteral("wayland-")))
        return SurfacePlatform::Wayland;
    if (name == QStringLiteral("xcb"))
        return SurfacePlatform::X11;
    if (name == QStringLiteral("offscreen") || name == QStringLiteral("minimal"))
        return SurfacePlatform::Offscreen;
    return SurfacePlatform::Unknown;
}

std::unique_ptr<SurfaceBackend> ShellSurfacePrivate::createBackend()
{
    return SurfaceBackend::create();
}

// ── ShellSurface ───────────────────────────────────────────

ShellSurface::ShellSurface(ShellSurfacePrivate *d, QObject *parent)
    : QObject(parent)
    , d(d)
{
    Q_ASSERT(d);

    if (!d->backend)
        d->backend = d->createBackend();

    // Track screen added/removed so we can move the surface if needed.
    connect(QGuiApplication::instance(), &QGuiApplication::screenRemoved,
            this, [this](QScreen *removed) {
                if (d->screen == removed) {
                    setScreen(QGuiApplication::primaryScreen());
                }
            });
}

ShellSurface::~ShellSurface()
{
    destroyWindow();
}

bool ShellSurface::isVisible() const
{
    return d->visible;
}

QScreen *ShellSurface::screen() const
{
    return d->screen;
}

void ShellSurface::setScreen(QScreen *screen)
{
    if (d->screen == screen)
        return;

    d->screen = screen;
    Q_EMIT screenChanged(screen);

    if (d->visible)
        reload();
}

SurfacePlatform ShellSurface::platform() const
{
    return d->platform;
}

QWindow *ShellSurface::window() const
{
    return d->window;
}

void ShellSurface::show()
{
    if (d->visible)
        return;

    if (!d->window)
        createWindow();

    if (d->window) {
        d->window->show();
        d->visible = true;
        Q_EMIT visibilityChanged(true);
        qCDebug(lcShellSurface) << metaObject()->className() << "shown";
    }
}

void ShellSurface::hide()
{
    if (!d->visible)
        return;

    if (d->window)
        d->window->hide();

    d->visible = false;
    Q_EMIT visibilityChanged(false);
    qCDebug(lcShellSurface) << metaObject()->className() << "hidden";
}

void ShellSurface::reload()
{
    const bool wasVisible = d->visible;
    destroyWindow();
    if (wasVisible)
        show();
}

void ShellSurface::destroyWindow()
{
    if (!d->window)
        return;

    d->window->hide();
    if (d->backend)
        d->backend->destroyLayerWindow(d->window);
    else
        delete d->window;

    d->window = nullptr;
    d->visible = false;
}

} // namespace Lingmo
