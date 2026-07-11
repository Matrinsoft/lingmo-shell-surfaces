#include "ForeignToplevelHandle.h"

#include <QLoggingCategory>
#include <cstring>

Q_LOGGING_CATEGORY(lcForeignHandle, "lingmo.shell.surfaces.foreigntoplevel.handle")

namespace Lingmo {

ForeignToplevelHandle::ForeignToplevelHandle(
    struct ::zwlr_foreign_toplevel_handle_v1 *handle,
    QObject *parent)
    : QObject(parent)
    , QtWayland::zwlr_foreign_toplevel_handle_v1(handle)
{
    // Use the raw pointer address as a stable unique id for this handle.
    id = QStringLiteral("wl-%1").arg(reinterpret_cast<quintptr>(handle));
}

ForeignToplevelHandle::~ForeignToplevelHandle()
{
    if (isInitialized())
        destroy();
}

void ForeignToplevelHandle::requestActivate()
{
    // Protocol requires a wl_seat; pass nullptr — KWin accepts it.
    activate(nullptr);
}

void ForeignToplevelHandle::requestClose()
{
    close();
}

// ── Protocol event handlers ───────────────────────────────────────────────────

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_title(const QString &title)
{
    currentTitle = title;
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_app_id(const QString &app_id)
{
    currentAppId = app_id;
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_state(wl_array *stateArray)
{
    states.clear();
    // stateArray->data is a packed array of uint32_t state enum values.
    const uint32_t *begin = static_cast<const uint32_t *>(stateArray->data);
    const uint32_t *end   = begin + stateArray->size / sizeof(uint32_t);
    for (const uint32_t *p = begin; p < end; ++p) {
        switch (*p) {
        case 0: states.insert(State::maximized);  break;
        case 1: states.insert(State::minimized);  break;
        case 2: states.insert(State::activated);  break;
        case 3: states.insert(State::fullscreen); break;
        default: break;
        }
    }
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_done()
{
    if (!initialised) {
        initialised = true;
        qCDebug(lcForeignHandle) << "Handle ready:" << id << currentTitle << currentAppId;
        Q_EMIT ready();
    } else {
        Q_EMIT changed();
    }
}

void ForeignToplevelHandle::zwlr_foreign_toplevel_handle_v1_closed()
{
    qCDebug(lcForeignHandle) << "Handle closed:" << id;
    Q_EMIT closed();
}

} // namespace Lingmo
