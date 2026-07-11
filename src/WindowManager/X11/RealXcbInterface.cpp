#include "RealXcbInterface.h"

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

#include <QGuiApplication>
#include <QLoggingCategory>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QNativeInterface>
#endif

Q_LOGGING_CATEGORY(lcXcb, "lingmo.shell.surfaces.x11.xcb")

namespace Lingmo {

// ── Factory ───────────────────────────────────────────────────────────────────

RealXcbInterface *RealXcbInterface::create()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        qCDebug(lcXcb) << "Not running on X11";
        return nullptr;
    }
    xcb_connection_t *conn = x11App->connection();
    if (!conn || xcb_connection_has_error(conn)) {
        qCWarning(lcXcb) << "XCB connection has error";
        return nullptr;
    }
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    const XcbWindow root = static_cast<XcbWindow>(it.data->root);
    return new RealXcbInterface(conn, root);
#else
    return nullptr;
#endif
}

// ── Constructor ───────────────────────────────────────────────────────────────

RealXcbInterface::RealXcbInterface(xcb_connection_t *conn, XcbWindow root)
    : conn(conn), root(root)
{
}

// ── XcbInterface implementation ───────────────────────────────────────────────

int RealXcbInterface::connectionFd() const
{
    return xcb_get_file_descriptor(conn);
}

bool RealXcbInterface::isValid() const
{
    return conn && !xcb_connection_has_error(conn);
}

XcbAtom RealXcbInterface::internAtom(const char *name) const
{
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(conn, 0, static_cast<uint16_t>(strlen(name)), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, nullptr);
    if (!reply)
        return 0;
    const XcbAtom atom = reply->atom;
    free(reply);
    return atom;
}

XcbPropertyReply RealXcbInterface::getProperty(
    XcbWindow window, XcbAtom property, XcbAtom type, uint32_t length) const
{
    XcbPropertyReply result;

    xcb_get_property_cookie_t cookie =
        xcb_get_property(conn, 0, window, property, type, 0, length);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, nullptr);
    if (!reply)
        return result;

    const int len = xcb_get_property_value_length(reply);
    if (len > 0) {
        result.valid  = true;
        result.type   = reply->type;
        result.format = reply->format;
        result.data   = QByteArray(
            reinterpret_cast<const char *>(xcb_get_property_value(reply)), len);
    }
    free(reply);
    return result;
}

void RealXcbInterface::sendClientMessage(
    XcbWindow destination, XcbAtom messageType,
    uint32_t data0, uint32_t data1, uint32_t data2,
    uint32_t data3, uint32_t data4)
{
    xcb_client_message_event_t ev{};
    ev.response_type  = XCB_CLIENT_MESSAGE;
    ev.format         = 32;
    ev.sequence       = 0;
    ev.window         = destination;
    ev.type           = messageType;
    ev.data.data32[0] = data0;
    ev.data.data32[1] = data1;
    ev.data.data32[2] = data2;
    ev.data.data32[3] = data3;
    ev.data.data32[4] = data4;

    const uint32_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_send_event(conn, 0, root, mask,
                   reinterpret_cast<const char *>(&ev));
}

void RealXcbInterface::flush()
{
    xcb_flush(conn);
}

void RealXcbInterface::subscribePropertyNotify(XcbWindow window)
{
    const uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &mask);
    xcb_flush(conn);
}

QList<XcbInterface::PendingEvent> RealXcbInterface::pollEvents()
{
    QList<PendingEvent> events;
    while (xcb_generic_event_t *raw = xcb_poll_for_event(conn)) {
        const uint8_t type = XCB_EVENT_RESPONSE_TYPE(raw);
        if (type == XCB_PROPERTY_NOTIFY) {
            auto *pn = reinterpret_cast<xcb_property_notify_event_t *>(raw);
            events.append(PendingEvent{type, pn->window, pn->atom});
        }
        free(raw);
    }
    return events;
}

} // namespace Lingmo
