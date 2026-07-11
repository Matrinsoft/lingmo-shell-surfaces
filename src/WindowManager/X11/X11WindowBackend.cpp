#include "X11WindowBackend.h"
#include "RealXcbInterface.h"

#include <xcb/xcb.h>

#include <QLoggingCategory>
#include <QSocketNotifier>

Q_LOGGING_CATEGORY(lcX11Back, "lingmo.shell.surfaces.x11.backend")

namespace Lingmo {

// ── Constructor / destructor ──────────────────────────────────────────────────

X11WindowBackend::X11WindowBackend(std::unique_ptr<XcbInterface> xcbIface,
                                    QObject *parent)
    : WindowManagerBackend(parent)
    , xcb(std::move(xcbIface))
{
}

X11WindowBackend::~X11WindowBackend() = default;

// ── Initialize ────────────────────────────────────────────────────────────────

void X11WindowBackend::initialize()
{
    if (!xcb)
        xcb.reset(RealXcbInterface::create());

    if (!xcb || !xcb->isValid()) {
        qCWarning(lcX11Back) << "XCB connection unavailable — X11 backend disabled";
        Q_EMIT availabilityChanged(false);
        return;
    }

    internAtoms();

    // Watch root window for property changes (_NET_CLIENT_LIST, _NET_ACTIVE_WINDOW, …)
    xcb->subscribePropertyNotify(xcb->rootWindow());
    xcb->flush();

    // Drive XCB event processing from Qt's event loop.
    const int fd = xcb->connectionFd();
    if (fd >= 0) {
        notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this,
                &X11WindowBackend::onXcbEvent);
    }

    refreshWorkspaceList();
    refreshWindowList();

    available = true;
    Q_EMIT availabilityChanged(true);
    qCInfo(lcX11Back) << "X11 backend ready:"
                      << windowList.size() << "windows,"
                      << workspaceList.size() << "workspaces";
}

// ── Atom cache ────────────────────────────────────────────────────────────────

void X11WindowBackend::internAtoms()
{
    atoms.netClientList       = xcb->internAtom("_NET_CLIENT_LIST");
    atoms.netClientListStack  = xcb->internAtom("_NET_CLIENT_LIST_STACKING");
    atoms.netActiveWindow     = xcb->internAtom("_NET_ACTIVE_WINDOW");
    atoms.netWmName           = xcb->internAtom("_NET_WM_NAME");
    atoms.wmName              = xcb->internAtom("WM_NAME");     // Latin-1 fallback
    atoms.netWmPid            = xcb->internAtom("_NET_WM_PID");
    atoms.netWmDesktop        = xcb->internAtom("_NET_WM_DESKTOP");
    atoms.netWmState          = xcb->internAtom("_NET_WM_STATE");
    atoms.netWmStateHidden    = xcb->internAtom("_NET_WM_STATE_HIDDEN");
    atoms.netWmStateMaxHorz   = xcb->internAtom("_NET_WM_STATE_MAXIMIZED_HORZ");
    atoms.netWmStateMaxVert   = xcb->internAtom("_NET_WM_STATE_MAXIMIZED_VERT");
    atoms.netNumberOfDesktops = xcb->internAtom("_NET_NUMBER_OF_DESKTOPS");
    atoms.netCurrentDesktop   = xcb->internAtom("_NET_CURRENT_DESKTOP");
    atoms.netDesktopNames     = xcb->internAtom("_NET_DESKTOP_NAMES");
    atoms.netCloseWindow      = xcb->internAtom("_NET_CLOSE_WINDOW");
    atoms.wmClass             = xcb->internAtom("WM_CLASS");
    atoms.utf8String          = xcb->internAtom("UTF8_STRING");
    // Pre-defined XCB atoms — intern for symmetry so MockXcbInterface can seed them.
    atoms.atom                = xcb->internAtom("ATOM");
    atoms.cardinal            = xcb->internAtom("CARDINAL");
    atoms.window              = xcb->internAtom("WINDOW");
}

// ── Window list refresh ───────────────────────────────────────────────────────

void X11WindowBackend::refreshWindowList()
{
    // Prefer stacking order for Overview display.
    XcbPropertyReply stackReply = xcb->getProperty(
        xcb->rootWindow(), atoms.netClientListStack, XCB_ATOM_WINDOW, 1024);
    if (!stackReply.valid)
        stackReply = xcb->getProperty(
            xcb->rootWindow(), atoms.netClientList, XCB_ATOM_WINDOW, 1024);

    // Identify the active window.
    XcbPropertyReply activeReply = xcb->getProperty(
        xcb->rootWindow(), atoms.netActiveWindow, XCB_ATOM_WINDOW, 1);
    activeXid = 0;
    if (activeReply.valid && activeReply.data.size() >= 4)
        activeXid = *reinterpret_cast<const uint32_t *>(activeReply.data.constData());

    windowList.clear();
    if (!stackReply.valid)
        return;

    const int count = stackReply.data.size() / 4;
    windowList.reserve(count);
    const auto *xids = reinterpret_cast<const uint32_t *>(stackReply.data.constData());
    for (int i = 0; i < count; ++i) {
        const XcbWindow xid = xids[i];
        WindowInfo w = buildWindowInfo(xid);
        w.isActive = (xid == activeXid);
        windowList.append(w);
    }

    qCDebug(lcX11Back) << "Window list refreshed:" << windowList.size() << "windows";
    Q_EMIT windowsChanged();
}

// ── Per-window property fetch ─────────────────────────────────────────────────

WindowInfo X11WindowBackend::buildWindowInfo(XcbWindow xid) const
{
    WindowInfo w;
    w.windowId = QStringLiteral("x11-%1").arg(xid);

    // ① _NET_WM_NAME (UTF-8, preferred)
    XcbPropertyReply nameReply = xcb->getProperty(xid, atoms.netWmName, atoms.utf8String, 512);
    if (nameReply.valid)
        w.title = QString::fromUtf8(nameReply.data);

    // ② WM_NAME (Latin-1 fallback) — uses the interned atom, NOT the non-existent
    //    XCB_ATOM_WM_NAME constant.
    if (w.title.isEmpty()) {
        XcbPropertyReply wmNameReply =
            xcb->getProperty(xid, atoms.wmName, XCB_ATOM_STRING, 512);
        if (wmNameReply.valid)
            w.title = QString::fromLatin1(wmNameReply.data);
    }

    // WM_CLASS → appId (instance\0class\0 — take the class part)
    XcbPropertyReply classReply =
        xcb->getProperty(xid, atoms.wmClass, XCB_ATOM_STRING, 256);
    if (classReply.valid && !classReply.data.isEmpty()) {
        const QByteArrayList parts = classReply.data.split('\0');
        if (parts.size() >= 2 && !parts.at(1).isEmpty())
            w.appId = QString::fromLatin1(parts.at(1));
        else if (!parts.isEmpty())
            w.appId = QString::fromLatin1(parts.at(0));
    }

    // _NET_WM_DESKTOP
    XcbPropertyReply desktopReply =
        xcb->getProperty(xid, atoms.netWmDesktop, XCB_ATOM_CARDINAL, 1);
    if (desktopReply.valid && desktopReply.data.size() >= 4) {
        const uint32_t desktop =
            *reinterpret_cast<const uint32_t *>(desktopReply.data.constData());
        // 0xFFFFFFFF = sticky (all desktops) → place on current desktop.
        w.workspaceIndex = (desktop == 0xFFFFFFFFu)
                           ? currentWsIdx
                           : static_cast<int>(desktop);
    } else {
        w.workspaceIndex = currentWsIdx;
    }

    // _NET_WM_STATE
    XcbPropertyReply stateReply =
        xcb->getProperty(xid, atoms.netWmState, XCB_ATOM_ATOM, 32);
    if (stateReply.valid) {
        const int atomCount = stateReply.data.size() / 4;
        const auto *stateAtoms =
            reinterpret_cast<const uint32_t *>(stateReply.data.constData());
        for (int i = 0; i < atomCount; ++i) {
            if (stateAtoms[i] == atoms.netWmStateHidden)
                w.isMinimized = true;
        }
    }

    return w;
}

// ── Workspace list refresh ────────────────────────────────────────────────────

void X11WindowBackend::refreshWorkspaceList()
{
    // Number of desktops
    int count = 1;
    XcbPropertyReply countReply = xcb->getProperty(
        xcb->rootWindow(), atoms.netNumberOfDesktops, XCB_ATOM_CARDINAL, 1);
    if (countReply.valid && countReply.data.size() >= 4)
        count = static_cast<int>(
            *reinterpret_cast<const uint32_t *>(countReply.data.constData()));

    // Current desktop index
    XcbPropertyReply curReply = xcb->getProperty(
        xcb->rootWindow(), atoms.netCurrentDesktop, XCB_ATOM_CARDINAL, 1);
    if (curReply.valid && curReply.data.size() >= 4)
        currentWsIdx = static_cast<int>(
            *reinterpret_cast<const uint32_t *>(curReply.data.constData()));

    // Desktop names (_NET_DESKTOP_NAMES, UTF-8 NUL-separated)
    QStringList names;
    XcbPropertyReply namesReply = xcb->getProperty(
        xcb->rootWindow(), atoms.netDesktopNames, atoms.utf8String, 2048);
    if (namesReply.valid) {
        QByteArray raw = namesReply.data;
        if (!raw.isEmpty() && raw.back() == '\0')
            raw.chop(1);
        for (const QByteArray &chunk : raw.split('\0'))
            names.append(QString::fromUtf8(chunk));
    }

    workspaceList.clear();
    workspaceList.reserve(count);
    for (int i = 0; i < count; ++i) {
        WorkspaceInfo ws;
        ws.workspaceIndex = i;
        ws.name = (i < names.size() && !names.at(i).isEmpty())
                  ? names.at(i)
                  : QStringLiteral("Desktop %1").arg(i + 1);
        ws.active = (i == currentWsIdx);
        workspaceList.append(ws);
    }

    qCDebug(lcX11Back) << "Workspace list refreshed:" << workspaceList.size()
                       << "desktops; current =" << currentWsIdx;
    Q_EMIT workspacesChanged();
    Q_EMIT currentWorkspaceIndexChanged(currentWsIdx);
}

// ── XCB event processing ──────────────────────────────────────────────────────

void X11WindowBackend::onXcbEvent()
{
    const QList<XcbInterface::PendingEvent> events = xcb->pollEvents();
    bool windowsDirty    = false;
    bool workspacesDirty = false;

    for (const auto &ev : events) {
        if (ev.window != xcb->rootWindow())
            continue;
        if (ev.atom == atoms.netClientList || ev.atom == atoms.netClientListStack
                || ev.atom == atoms.netActiveWindow)
            windowsDirty = true;
        else if (ev.atom == atoms.netCurrentDesktop
                 || ev.atom == atoms.netNumberOfDesktops
                 || ev.atom == atoms.netDesktopNames)
            workspacesDirty = true;
    }

    if (workspacesDirty)
        refreshWorkspaceList();
    if (windowsDirty)
        refreshWindowList();
}

// ── Accessors ─────────────────────────────────────────────────────────────────

QList<WindowInfo> X11WindowBackend::windows() const
{
    return windowList;
}

QList<WorkspaceInfo> X11WindowBackend::workspaces() const
{
    return workspaceList;
}

int X11WindowBackend::currentWorkspaceIndex() const
{
    return currentWsIdx;
}

bool X11WindowBackend::isAvailable() const
{
    return available;
}

// ── Private helpers ───────────────────────────────────────────────────────────

// Parses "x11-<xid>" → xid.  Returns 0 on any failure.
XcbWindow X11WindowBackend::parseWindowId(const QString &windowId)
{
    constexpr QStringView prefix = u"x11-";
    if (!QStringView{windowId}.startsWith(prefix))
        return 0;
    bool ok = false;
    const XcbWindow xid = QStringView{windowId}.mid(prefix.size()).toUInt(&ok);
    return ok ? xid : 0;
}

// Sends _NET_WM_STATE client message.
// action: 0=remove, 1=add, 2=toggle
void X11WindowBackend::sendNetWmState(XcbWindow win, int action,
                                       XcbAtom atom1, XcbAtom atom2)
{
    // data[0]=action, data[1]=property1, data[2]=property2, data[3]=source (2=pager)
    xcb->sendClientMessage(win, atoms.netWmState,
                           static_cast<uint32_t>(action),
                           atom1, atom2 ? atom2 : 0, 2, 0);
    xcb->flush();
}

// ── Actions ───────────────────────────────────────────────────────────────────

void X11WindowBackend::activateWindow(const QString &windowId)
{
    if (!available)
        return;
    const XcbWindow xid = parseWindowId(windowId);
    if (!xid)
        return;
    // _NET_ACTIVE_WINDOW: data[0]=source(2=pager), data[1]=timestamp(0), data[2]=current(0)
    xcb->sendClientMessage(xid, atoms.netActiveWindow, 2, 0, 0);
    xcb->flush();
}

void X11WindowBackend::closeWindow(const QString &windowId)
{
    if (!available)
        return;
    const XcbWindow xid = parseWindowId(windowId);
    if (!xid)
        return;
    // _NET_CLOSE_WINDOW: data[0]=timestamp(0), data[1]=source(2=pager)
    xcb->sendClientMessage(xid, atoms.netCloseWindow, 0, 2);
    xcb->flush();
}

void X11WindowBackend::minimizeWindow(const QString &windowId)
{
    if (!available)
        return;
    const XcbWindow xid = parseWindowId(windowId);
    if (!xid)
        return;
    // Add _NET_WM_STATE_HIDDEN (minimized state)
    sendNetWmState(xid, 1 /* add */, atoms.netWmStateHidden);
}

void X11WindowBackend::maximizeWindow(const QString &windowId)
{
    if (!available)
        return;
    const XcbWindow xid = parseWindowId(windowId);
    if (!xid)
        return;
    // Add both _NET_WM_STATE_MAXIMIZED_HORZ and _NET_WM_STATE_MAXIMIZED_VERT
    sendNetWmState(xid, 1 /* add */, atoms.netWmStateMaxHorz, atoms.netWmStateMaxVert);
}

void X11WindowBackend::switchToWorkspace(int workspaceIndex)
{
    if (!available)
        return;
    // _NET_CURRENT_DESKTOP: data[0]=desktop, data[1]=timestamp
    xcb->sendClientMessage(xcb->rootWindow(), atoms.netCurrentDesktop,
                           static_cast<uint32_t>(workspaceIndex), 0);
    xcb->flush();
    currentWsIdx = workspaceIndex;
    Q_EMIT currentWorkspaceIndexChanged(workspaceIndex);
}

} // namespace Lingmo
