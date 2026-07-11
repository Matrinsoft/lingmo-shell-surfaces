#include "WindowManagerBackend.h"
#include "KWinBackend.h"

namespace Lingmo {

WindowManagerBackend::WindowManagerBackend(QObject *parent)
    : QObject(parent)
{
}

WindowManagerBackend::~WindowManagerBackend() = default;

WindowManagerBackend *WindowManagerBackend::create(QObject *parent)
{
    return new KWinBackend(parent);
}

} // namespace Lingmo
