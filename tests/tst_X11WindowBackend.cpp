#include <QTest>
#include <QSignalSpy>

#include "../src/WindowManager/X11/X11WindowBackend.h"
#include "MockXcbInterface.h"

using namespace Lingmo;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Atom ID constants used across all tests.
enum TestAtom : uint32_t {
    A_NET_CLIENT_LIST         = 101,
    A_NET_CLIENT_LIST_STACKING = 102,
    A_NET_ACTIVE_WINDOW       = 103,
    A_NET_WM_NAME             = 104,
    A_NET_WM_PID              = 105,
    A_NET_WM_DESKTOP          = 106,
    A_NET_WM_STATE            = 107,
    A_NET_WM_STATE_HIDDEN     = 108,
    A_NET_WM_STATE_MAX_HORZ   = 109,
    A_NET_WM_STATE_MAX_VERT   = 110,
    A_NET_NUMBER_OF_DESKTOPS  = 111,
    A_NET_CURRENT_DESKTOP     = 112,
    A_NET_DESKTOP_NAMES       = 113,
    A_NET_CLOSE_WINDOW        = 114,
    A_WM_CLASS                = 115,
    A_UTF8_STRING             = 116,
    A_NET_WM_STATE_ACTION     = 117,
    A_ATOM                    = 118,
    A_CARDINAL                = 119,
    A_WINDOW                  = 120,
    A_WM_NAME                 = 121,
};

static std::unique_ptr<MockXcbInterface> makeMock()
{
    auto mock = std::make_unique<MockXcbInterface>();

    mock->setAtom("_NET_CLIENT_LIST",           A_NET_CLIENT_LIST);
    mock->setAtom("_NET_CLIENT_LIST_STACKING",  A_NET_CLIENT_LIST_STACKING);
    mock->setAtom("_NET_ACTIVE_WINDOW",         A_NET_ACTIVE_WINDOW);
    mock->setAtom("_NET_WM_NAME",               A_NET_WM_NAME);
    mock->setAtom("_NET_WM_PID",                A_NET_WM_PID);
    mock->setAtom("_NET_WM_DESKTOP",            A_NET_WM_DESKTOP);
    mock->setAtom("_NET_WM_STATE",              A_NET_WM_STATE);
    mock->setAtom("_NET_WM_STATE_HIDDEN",       A_NET_WM_STATE_HIDDEN);
    mock->setAtom("_NET_WM_STATE_MAXIMIZED_HORZ", A_NET_WM_STATE_MAX_HORZ);
    mock->setAtom("_NET_WM_STATE_MAXIMIZED_VERT", A_NET_WM_STATE_MAX_VERT);
    mock->setAtom("_NET_NUMBER_OF_DESKTOPS",    A_NET_NUMBER_OF_DESKTOPS);
    mock->setAtom("_NET_CURRENT_DESKTOP",       A_NET_CURRENT_DESKTOP);
    mock->setAtom("_NET_DESKTOP_NAMES",         A_NET_DESKTOP_NAMES);
    mock->setAtom("_NET_CLOSE_WINDOW",          A_NET_CLOSE_WINDOW);
    mock->setAtom("WM_CLASS",                   A_WM_CLASS);
    mock->setAtom("UTF8_STRING",                A_UTF8_STRING);
    mock->setAtom("WM_NAME",                    A_WM_NAME);
    mock->setAtom("ATOM",                       A_ATOM);
    mock->setAtom("CARDINAL",                   A_CARDINAL);
    mock->setAtom("WINDOW",                     A_WINDOW);

    // Default: 2 workspaces, current = 0
    mock->setCardinals(MockXcbInterface::RootWindow, A_NET_NUMBER_OF_DESKTOPS, {2});
    mock->setCardinals(MockXcbInterface::RootWindow, A_NET_CURRENT_DESKTOP,    {0});

    // Desktop names as NUL-separated UTF-8
    QByteArray names = "Home\0Work";
    names.append('\0');
    mock->setProperty(MockXcbInterface::RootWindow, A_NET_DESKTOP_NAMES,
                      A_UTF8_STRING, 8, names);

    return mock;
}

static void seedTwoWindows(MockXcbInterface *mock)
{
    // _NET_CLIENT_LIST_STACKING: windows 10 and 20
    mock->setWindows(MockXcbInterface::RootWindow, A_NET_CLIENT_LIST_STACKING, {10, 20});

    // Active window: 10
    mock->setCardinals(MockXcbInterface::RootWindow, A_NET_ACTIVE_WINDOW, {10});

    // Window 10: title = "Terminal", appId = "Konsole", desktop 0
    mock->setUtf8(10, A_NET_WM_NAME, QStringLiteral("Terminal"));
    mock->setProperty(10, A_WM_CLASS, XCB_ATOM_STRING, 8,
                      QByteArray("konsole\0Konsole\0", 16));
    mock->setCardinals(10, A_NET_WM_DESKTOP, {0});

    // Window 20: title = "Files", appId = "Dolphin", desktop 1, minimized
    mock->setUtf8(20, A_NET_WM_NAME, QStringLiteral("Files"));
    mock->setProperty(20, A_WM_CLASS, XCB_ATOM_STRING, 8,
                      QByteArray("dolphin\0Dolphin\0", 16));
    mock->setCardinals(20, A_NET_WM_DESKTOP, {1});
    mock->setAtomList(20, A_NET_WM_STATE, {A_NET_WM_STATE_HIDDEN});
}

// ── Test class ────────────────────────────────────────────────────────────────

class tst_X11WindowBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // ── EWMH atom parsing ────────────────────────────────────────────────────
    void testAtomInternedOnInit()
    {
        auto mock = makeMock();
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QVERIFY(backend.isAvailable());
        // Atoms used internally should be resolved (all non-zero).
        // Verify indirectly by checking that refreshes succeed.
        QVERIFY(!backend.workspaces().isEmpty());
    }

    // ── Window list ──────────────────────────────────────────────────────────
    void testWindowListPopulated()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.windows().size(), 2);
    }

    void testWindowIdFormat()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        const auto wins = backend.windows();
        QCOMPARE(wins.at(0).windowId, QStringLiteral("x11-10"));
        QCOMPARE(wins.at(1).windowId, QStringLiteral("x11-20"));
    }

    void testWindowTitle()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.windows().at(0).title, QStringLiteral("Terminal"));
        QCOMPARE(backend.windows().at(1).title, QStringLiteral("Files"));
    }

    void testWindowAppId()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.windows().at(0).appId, QStringLiteral("Konsole"));
        QCOMPARE(backend.windows().at(1).appId, QStringLiteral("Dolphin"));
    }

    void testActiveWindow()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QVERIFY( backend.windows().at(0).isActive);  // XID 10 is active
        QVERIFY(!backend.windows().at(1).isActive);  // XID 20 is not
    }

    void testMinimizedState()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QVERIFY(!backend.windows().at(0).isMinimized);
        QVERIFY( backend.windows().at(1).isMinimized); // window 20 has STATE_HIDDEN
    }

    void testWindowDesktopMapping()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.windows().at(0).workspaceIndex, 0);
        QCOMPARE(backend.windows().at(1).workspaceIndex, 1);
    }

    void testEmptyWindowListSignal()
    {
        auto mock = makeMock();
        // No windows seeded
        X11WindowBackend backend(std::move(mock));
        QSignalSpy spy(&backend, &WindowManagerBackend::windowsChanged);
        backend.initialize();

        QCOMPARE(spy.count(), 1);
        QVERIFY(backend.windows().isEmpty());
    }

    // ── Workspace list ───────────────────────────────────────────────────────
    void testWorkspaceCount()
    {
        auto mock = makeMock();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.workspaces().size(), 2);
    }

    void testWorkspaceNames()
    {
        auto mock = makeMock();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.workspaces().at(0).name, QStringLiteral("Home"));
        QCOMPARE(backend.workspaces().at(1).name, QStringLiteral("Work"));
    }

    void testCurrentWorkspaceIndex()
    {
        auto mock = makeMock();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QCOMPARE(backend.currentWorkspaceIndex(), 0);
    }

    void testWorkspaceActiveFlag()
    {
        auto mock = makeMock();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QVERIFY( backend.workspaces().at(0).active);
        QVERIFY(!backend.workspaces().at(1).active);
    }

    // ── Actions ──────────────────────────────────────────────────────────────
    void testActivateWindow()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        raw->sentMessages.clear();
        backend.activateWindow(QStringLiteral("x11-10"));

        QCOMPARE(raw->sentMessages.size(), 1);
        QCOMPARE(raw->sentMessages.first().dest, XcbWindow(10));
        QCOMPARE(raw->sentMessages.first().type, XcbAtom(A_NET_ACTIVE_WINDOW));
    }

    void testActivateWindowInvalidId()
    {
        auto mock = makeMock();
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        raw->sentMessages.clear();
        backend.activateWindow(QStringLiteral("unknown-id"));
        QVERIFY(raw->sentMessages.isEmpty());
    }

    void testCloseWindow()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        raw->sentMessages.clear();
        backend.closeWindow(QStringLiteral("x11-20"));

        QCOMPARE(raw->sentMessages.size(), 1);
        QCOMPARE(raw->sentMessages.first().dest, XcbWindow(20));
        QCOMPARE(raw->sentMessages.first().type, XcbAtom(A_NET_CLOSE_WINDOW));
    }

    void testMinimizeWindow()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        raw->sentMessages.clear();
        backend.minimizeWindow(QStringLiteral("x11-10"));

        // Expect exactly one _NET_WM_STATE message with action=1 (add) and
        // first property = _NET_WM_STATE_HIDDEN.
        QCOMPARE(raw->sentMessages.size(), 1);
        const auto &msg = raw->sentMessages.first();
        QCOMPARE(msg.type, XcbAtom(A_NET_WM_STATE));
        QCOMPARE(msg.data[0], uint32_t(1));            // action = add
        QCOMPARE(msg.data[1], XcbAtom(A_NET_WM_STATE_HIDDEN));
    }

    void testMaximizeWindow()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        raw->sentMessages.clear();
        backend.maximizeWindow(QStringLiteral("x11-10"));

        // Expect exactly one _NET_WM_STATE message with action=1 (add) and
        // both _NET_WM_STATE_MAXIMIZED_HORZ and _NET_WM_STATE_MAXIMIZED_VERT.
        QCOMPARE(raw->sentMessages.size(), 1);
        const auto &msg = raw->sentMessages.first();
        QCOMPARE(msg.type, XcbAtom(A_NET_WM_STATE));
        QCOMPARE(msg.data[0], uint32_t(1));            // action = add
        QCOMPARE(msg.data[1], XcbAtom(A_NET_WM_STATE_MAX_HORZ));
        QCOMPARE(msg.data[2], XcbAtom(A_NET_WM_STATE_MAX_VERT));
    }

    void testSwitchToWorkspace()
    {
        auto mock = makeMock();
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        QSignalSpy spy(&backend, &WindowManagerBackend::currentWorkspaceIndexChanged);
        backend.initialize();

        raw->sentMessages.clear();
        spy.clear();
        backend.switchToWorkspace(1);

        QCOMPARE(raw->sentMessages.size(), 1);
        QCOMPARE(raw->sentMessages.first().type, XcbAtom(A_NET_CURRENT_DESKTOP));
        QCOMPARE(raw->sentMessages.first().data[0], uint32_t(1));
        QCOMPARE(backend.currentWorkspaceIndex(), 1);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toInt(), 1);
    }

    // ── Property-notify event handling ───────────────────────────────────────
    void testPropertyNotifyRefreshesWindowList()
    {
        auto mock = makeMock();
        seedTwoWindows(mock.get());
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QSignalSpy spy(&backend, &WindowManagerBackend::windowsChanged);

        // Inject a PropertyNotify for _NET_CLIENT_LIST_STACKING on root.
        raw->injectEvent(XCB_PROPERTY_NOTIFY, MockXcbInterface::RootWindow,
                         A_NET_CLIENT_LIST_STACKING);

        // Since there is no file descriptor, call onXcbEvent() directly.
        QMetaObject::invokeMethod(&backend, "onXcbEvent");

        QCOMPARE(spy.count(), 1);
    }

    void testPropertyNotifyRefreshesWorkspaces()
    {
        auto mock = makeMock();
        auto *raw = mock.get();
        X11WindowBackend backend(std::move(mock));
        backend.initialize();

        QSignalSpy spy(&backend, &WindowManagerBackend::workspacesChanged);

        raw->injectEvent(XCB_PROPERTY_NOTIFY, MockXcbInterface::RootWindow,
                         A_NET_CURRENT_DESKTOP);
        QMetaObject::invokeMethod(&backend, "onXcbEvent");

        QCOMPARE(spy.count(), 1);
    }

    // ── Availability ─────────────────────────────────────────────────────────
    void testAvailableAfterInit()
    {
        auto mock = makeMock();
        X11WindowBackend backend(std::move(mock));
        QVERIFY(!backend.isAvailable());
        QSignalSpy spy(&backend, &WindowManagerBackend::availabilityChanged);
        backend.initialize();
        QVERIFY(backend.isAvailable());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toBool(), true);
    }
};

QTEST_GUILESS_MAIN(tst_X11WindowBackend)
#include "tst_X11WindowBackend.moc"
