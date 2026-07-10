#pragma once

#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QPointer>

class QScreen;
class QWindow;

namespace Lingmo {

class SurfaceBackend;

class ShellSurfacePrivate
{
public:
    explicit ShellSurfacePrivate(ShellSurface *q);
    virtual ~ShellSurfacePrivate();

    ShellSurface *q = nullptr;

    bool visible = false;
    QPointer<QScreen> screen;
    QWindow *window = nullptr;
    SurfacePlatform platform = SurfacePlatform::Unknown;

    std::unique_ptr<SurfaceBackend> backend;

    // Detects the running platform (Wayland/X11/Offscreen).
    static SurfacePlatform detectPlatform();

    // Creates the correct SurfaceBackend for the detected platform.
    std::unique_ptr<SurfaceBackend> createBackend();
};

} // namespace Lingmo

// Convenience cast helpers — replaces Q_D/Q_Q for our manual PIMPL.
#define LINGMO_D(Class) \
    Class##Private *const d = static_cast<Class##Private *>(this->d.get())
#define LINGMO_CD(Class) \
    const Class##Private *const d = static_cast<const Class##Private *>(this->d.get())
