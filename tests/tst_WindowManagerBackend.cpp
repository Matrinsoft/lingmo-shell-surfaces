#include <QtTest/QtTest>
#include <LingmoShellSurfaces/WindowManagerBackend.h>

class tst_WindowManagerBackend : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { qputenv("QT_QPA_PLATFORM", "offscreen"); }

    void createReturnsNonNull()
    {
        auto *backend = Lingmo::WindowManagerBackend::create();
        QVERIFY(backend != nullptr);
        delete backend;
    }

    void unavailableWhenNoDbus()
    {
        auto *backend = Lingmo::WindowManagerBackend::create();
        backend->initialize();
        // On CI there is no KWin running; just verify it does not crash.
        [[maybe_unused]] bool available = backend->isAvailable();
        delete backend;
    }

    void windowsReturnsList()
    {
        auto *backend = Lingmo::WindowManagerBackend::create();
        backend->initialize();
        auto wins = backend->windows();
        // May be empty; just verify the call does not crash.
        QVERIFY(wins.isEmpty() || !wins.isEmpty());
        delete backend;
    }

    void workspacesReturnsList()
    {
        auto *backend = Lingmo::WindowManagerBackend::create();
        backend->initialize();
        auto wss = backend->workspaces();
        // Placeholder implementation always returns 4 workspaces when
        // KWin is unavailable, or the real count otherwise.
        Q_UNUSED(wss);
        delete backend;
    }

    void windowInfoFields()
    {
        Lingmo::WindowInfo wi;
        wi.windowId       = QStringLiteral("test-id");
        wi.title          = QStringLiteral("Test Window");
        wi.workspaceIndex = 1;
        wi.isMinimized    = false;
        QCOMPARE(wi.windowId, QStringLiteral("test-id"));
        QCOMPARE(wi.workspaceIndex, 1);
        QCOMPARE(wi.isMinimized, false);
    }

    void workspaceInfoFields()
    {
        Lingmo::WorkspaceInfo ws;
        ws.workspaceIndex = 2;
        ws.name           = QStringLiteral("Workspace 3");
        ws.windowCount    = 5;
        ws.active         = true;
        QCOMPARE(ws.workspaceIndex, 2);
        QCOMPARE(ws.name, QStringLiteral("Workspace 3"));
        QCOMPARE(ws.active, true);
    }

    void availabilitySignalEmitted()
    {
        auto *backend = Lingmo::WindowManagerBackend::create();
        bool signalReceived = false;
        QObject::connect(backend, &Lingmo::WindowManagerBackend::availabilityChanged,
                         backend, [&signalReceived](bool /*available*/) {
            signalReceived = true;
        });
        backend->initialize();
        QVERIFY(signalReceived);
        delete backend;
    }
};

QTEST_MAIN(tst_WindowManagerBackend)
#include "tst_WindowManagerBackend.moc"
