#pragma once
#include "../src/WindowManager/X11/XcbInterface.h"

#include <QHash>
#include <QList>

namespace Lingmo {

// Test double for XcbInterface — no real X11 display needed.
// Seed properties with setProperty(); inject events with injectEvent().
class MockXcbInterface : public XcbInterface
{
public:
    static constexpr XcbWindow RootWindow = 1;

    // ── Seeding ───────────────────────────────────────────────────────────
    void setAtom(const char *name, XcbAtom atom)
    {
        atomMap[QByteArray(name)] = atom;
    }

    void setProperty(XcbWindow win, XcbAtom prop, XcbAtom type,
                     int format, const QByteArray &data)
    {
        PropertyKey key{win, prop};
        XcbPropertyReply r;
        r.valid  = true;
        r.type   = type;
        r.format = format;
        r.data   = data;
        properties[key] = r;
    }

    // Convenience: set a CARDINAL property from a list of uint32_t values.
    void setCardinals(XcbWindow win, XcbAtom prop,
                      const QList<uint32_t> &values)
    {
        QByteArray data;
        for (uint32_t v : values)
            data.append(reinterpret_cast<const char *>(&v), sizeof(v));
        setProperty(win, prop, XCB_ATOM_CARDINAL, 32, data);
    }

    // Convenience: set a WINDOW-list property.
    void setWindows(XcbWindow win, XcbAtom prop,
                    const QList<uint32_t> &xids)
    {
        QByteArray data;
        for (uint32_t xid : xids)
            data.append(reinterpret_cast<const char *>(&xid), sizeof(xid));
        setProperty(win, prop, XCB_ATOM_WINDOW, 32, data);
    }

    // Convenience: set a UTF-8 string property.
    void setUtf8(XcbWindow win, XcbAtom prop, const QString &value)
    {
        setProperty(win, prop, 31 /* UTF8_STRING atom */, 8,
                    value.toUtf8());
    }

    // Convenience: set an ATOM-list property.
    void setAtomList(XcbWindow win, XcbAtom prop,
                     const QList<uint32_t> &atomIds)
    {
        QByteArray data;
        for (uint32_t a : atomIds)
            data.append(reinterpret_cast<const char *>(&a), sizeof(a));
        setProperty(win, prop, XCB_ATOM_ATOM, 32, data);
    }

    void injectEvent(uint8_t type, XcbWindow window, XcbAtom atom)
    {
        pendingEvents.append(PendingEvent{type, window, atom});
    }

    // Recorded action calls
    struct ClientMsg { XcbWindow dest; XcbAtom type; uint32_t data[5]; };
    QList<ClientMsg> sentMessages;
    QList<XcbWindow> subscribedWindows;

    // ── XcbInterface ──────────────────────────────────────────────────────
    int       connectionFd() const override { return -1; }
    bool      isValid()      const override { return true; }
    XcbWindow rootWindow()   const override { return RootWindow; }

    XcbAtom internAtom(const char *name) const override
    {
        auto it = atomMap.find(QByteArray(name));
        return (it != atomMap.end()) ? it.value() : 0;
    }

    XcbPropertyReply getProperty(XcbWindow window, XcbAtom property,
                                  XcbAtom /*type*/, uint32_t /*length*/) const override
    {
        auto it = properties.find(PropertyKey{window, property});
        return (it != properties.end()) ? it.value() : XcbPropertyReply{};
    }

    void sendClientMessage(XcbWindow dest, XcbAtom messageType,
                            uint32_t d0, uint32_t d1, uint32_t d2,
                            uint32_t d3, uint32_t d4) override
    {
        sentMessages.append(ClientMsg{dest, messageType, {d0, d1, d2, d3, d4}});
    }

    void flush() override {}

    void subscribePropertyNotify(XcbWindow window) override
    {
        subscribedWindows.append(window);
    }

    QList<PendingEvent> pollEvents() override
    {
        QList<PendingEvent> ev = pendingEvents;
        pendingEvents.clear();
        return ev;
    }

private:
    struct PropertyKey {
        XcbWindow win;
        XcbAtom   prop;
        bool operator==(const PropertyKey &o) const
        { return win == o.win && prop == o.prop; }
    };
    struct PropertyKeyHash {
        size_t operator()(const PropertyKey &k) const
        { return qHash(k.win) ^ qHash(k.prop); }
    };

    QHash<QByteArray, XcbAtom> atomMap;
    QHash<PropertyKey, XcbPropertyReply, PropertyKeyHash> properties;
    QList<PendingEvent>        pendingEvents;
};

// Qt hash support for PropertyKey
inline size_t qHash(const MockXcbInterface::PropertyKey &k, size_t seed = 0)
{
    return qHash(k.win, seed) ^ qHash(k.prop, seed);
}

} // namespace Lingmo
