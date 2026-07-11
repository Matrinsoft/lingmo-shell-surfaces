#pragma once
#include "XcbInterface.h"

struct xcb_connection_t;

namespace Lingmo {

// Production XcbInterface backed by a real xcb_connection_t obtained from Qt.
class RealXcbInterface : public XcbInterface
{
public:
    // Acquires the XCB connection from QGuiApplication's native interface.
    // Returns nullptr if not running under X11.
    static RealXcbInterface *create();

    explicit RealXcbInterface(xcb_connection_t *conn, XcbWindow root);
    ~RealXcbInterface() override = default;

    [[nodiscard]] int       connectionFd() const override;
    [[nodiscard]] bool      isValid()      const override;
    [[nodiscard]] XcbWindow rootWindow()   const override { return root; }

    [[nodiscard]] XcbAtom internAtom(const char *name) const override;

    [[nodiscard]] XcbPropertyReply getProperty(
        XcbWindow window,
        XcbAtom   property,
        XcbAtom   type,
        uint32_t  length = 1024) const override;

    void sendClientMessage(
        XcbWindow destination,
        XcbAtom   messageType,
        uint32_t  data0,
        uint32_t  data1,
        uint32_t  data2,
        uint32_t  data3,
        uint32_t  data4) override;

    void flush() override;
    void subscribePropertyNotify(XcbWindow window) override;
    [[nodiscard]] QList<PendingEvent> pollEvents() override;

private:
    xcb_connection_t *conn;
    XcbWindow         root;
};

} // namespace Lingmo
