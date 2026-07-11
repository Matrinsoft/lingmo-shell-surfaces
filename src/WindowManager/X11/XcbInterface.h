#pragma once
// XcbInterface — thin abstraction over XCB calls used by X11WindowBackend.
// Keeping this as a pure interface makes X11WindowBackend testable without
// a real X11 display (inject MockXcbInterface in tests).

#include <QByteArray>
#include <QList>
#include <cstdint>

namespace Lingmo {

using XcbWindow = uint32_t;
using XcbAtom   = uint32_t;

struct XcbPropertyReply {
    bool        valid  = false;
    XcbAtom     type   = 0;
    int         format = 0;
    QByteArray  data;
};

// Minimal XCB interface needed for EWMH window management.
class XcbInterface
{
public:
    virtual ~XcbInterface() = default;

    // Returns the X11 file descriptor for use with QSocketNotifier.
    // Returns -1 if unavailable.
    [[nodiscard]] virtual int  connectionFd()                        const = 0;
    [[nodiscard]] virtual bool isValid()                             const = 0;
    [[nodiscard]] virtual XcbWindow rootWindow()                     const = 0;

    // Intern an atom by name.
    [[nodiscard]] virtual XcbAtom internAtom(const char *name)       const = 0;

    // Read a window property.
    [[nodiscard]] virtual XcbPropertyReply getProperty(
        XcbWindow window,
        XcbAtom   property,
        XcbAtom   type,
        uint32_t  length = 1024) const = 0;

    // Send a client message to the root window (used for actions).
    virtual void sendClientMessage(
        XcbWindow  destination,
        XcbAtom    messageType,
        uint32_t   data0,
        uint32_t   data1 = 0,
        uint32_t   data2 = 0,
        uint32_t   data3 = 0,
        uint32_t   data4 = 0) = 0;

    // Flush pending requests.
    virtual void flush() = 0;

    // Subscribe to PropertyNotify events on a window.
    virtual void subscribePropertyNotify(XcbWindow window) = 0;

    // Drain and return all pending events as raw data.
    // Returns list of {eventType (uint8_t), window (XcbWindow), atom (XcbAtom)}.
    struct PendingEvent {
        uint8_t   type;
        XcbWindow window;
        XcbAtom   atom;
    };
    [[nodiscard]] virtual QList<PendingEvent> pollEvents() = 0;
};

} // namespace Lingmo
