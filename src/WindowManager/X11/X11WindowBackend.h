#pragma once
#include "../WindowManagerBackend.h"
#include "XcbInterface.h"

#include <QHash>

class QSocketNotifier;

namespace Lingmo {

// X11 window management backend.
// Reads the window list from _NET_CLIENT_LIST, fetches per-window properties
// via EWMH atoms, and monitors the root window for PropertyNotify events.
// Internal -- not exported.
class X11WindowBackend : public WindowManagerBackend
{
    Q_OBJECT
public:
    // Takes ownership of xcbIface.  When nullptr, a RealXcbInterface is created
    // at initialize() time.
    explicit X11WindowBackend(std::unique_ptr<XcbInterface> xcbIface = nullptr,
                               QObject *parent = nullptr);
    ~X11WindowBackend() override;

    [[nodiscard]] QList<WindowInfo>    windows()             const override;
    [[nodiscard]] QList<WorkspaceInfo> workspaces()          const override;
    [[nodiscard]] int  currentWorkspaceIndex()               const override;
    [[nodiscard]] bool isAvailable()                         const override;

    void activateWindow(const QString &windowId)   override;
    void closeWindow(const QString &windowId)      override;
    void minimizeWindow(const QString &windowId)   override;
    void maximizeWindow(const QString &windowId)   override;
    void switchToWorkspace(int workspaceIndex)     override;
    void initialize()                              override;

private Q_SLOTS:
    void onXcbEvent();

private:
    struct AtomCache {
        XcbAtom netClientList       = 0;
        XcbAtom netClientListStack  = 0;
        XcbAtom netActiveWindow     = 0;
        XcbAtom netWmName           = 0;  // _NET_WM_NAME  (UTF-8)
        XcbAtom wmName              = 0;  // WM_NAME       (Latin-1 fallback)
        XcbAtom netWmPid            = 0;
        XcbAtom netWmDesktop        = 0;
        XcbAtom netWmState          = 0;
        XcbAtom netWmStateHidden    = 0;
        XcbAtom netWmStateMaxHorz   = 0;
        XcbAtom netWmStateMaxVert   = 0;
        XcbAtom netNumberOfDesktops = 0;
        XcbAtom netCurrentDesktop   = 0;
        XcbAtom netDesktopNames     = 0;
        XcbAtom netCloseWindow      = 0;
        XcbAtom wmClass             = 0;
        XcbAtom utf8String          = 0;
        XcbAtom atom                = 0;  // pre-defined XCB_ATOM_ATOM  (4)
        XcbAtom cardinal            = 0;  // pre-defined XCB_ATOM_CARDINAL (6)
        XcbAtom window              = 0;  // pre-defined XCB_ATOM_WINDOW (33)
    };

    void internAtoms();
    void refreshWindowList();
    void refreshWorkspaceList();
    WindowInfo buildWindowInfo(XcbWindow win) const;

    // Sends _NET_WM_STATE client message.
    // action: 0=remove, 1=add, 2=toggle
    void sendNetWmState(XcbWindow win, int action, XcbAtom atom1, XcbAtom atom2 = 0);

    // Parses "x11-<xid>" → xid.  Returns 0 on failure.
    static XcbWindow parseWindowId(const QString &windowId);

    std::unique_ptr<XcbInterface> xcb;
    QSocketNotifier              *notifier     = nullptr;
    AtomCache                     atoms;
    QList<WindowInfo>             windowList;
    QList<WorkspaceInfo>          workspaceList;
    XcbWindow                     activeXid    = 0;
    int                           currentWsIdx = 0;
    bool                          available    = false;
};

} // namespace Lingmo
