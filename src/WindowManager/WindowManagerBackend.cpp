#include "WindowManagerBackend.h"
#include "KWinBackend.h"

#ifdef LINGMO_HAVE_X11
#include "X11/X11WindowBackend.h"
#endif

#include <QGuiApplication>

namespace Lingmo {

WindowManagerBackend::WindowManagerBackend(QObject *parent)
    : QObject(parent)
{
}

WindowManagerBackend::~WindowManagerBackend() = default;

WindowManagerBackend *WindowManagerBackend::create(QObject *parent)
{
    const QString platform = QGuiApplication::platformName();

#ifdef LINGMO_HAVE_X11
    if (platform == QLatin1String("xcb")) {
        return new X11WindowBackend(nullptr, parent);
    }
#endif

    // Wayland (KWin): use KWin D-Bus + zwlr_foreign_toplevel
    return new KWinBackend(parent);
}

} // namespace Lingmo
