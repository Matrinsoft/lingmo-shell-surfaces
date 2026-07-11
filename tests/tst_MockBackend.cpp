#include <QTest>
#include <QSignalSpy>

#include "MockWindowManagerBackend.h"

using namespace Lingmo;

class tst_MockBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInitialState()
    {
        MockWindowManagerBackend b;
        QVERIFY(!b.isAvailable());
        QVERIFY(b.windows().isEmpty());
        QVERIFY(b.workspaces().isEmpty());
        QCOMPARE(b.currentWorkspaceIndex(), 0);
    }

    void testInitialize()
    {
        MockWindowManagerBackend b;
        QSignalSpy spy(&b, &WindowManagerBackend::availabilityChanged);
        b.initialize();
        QVERIFY(b.isAvailable());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toBool(), true);
    }

    void testSetWindows()
    {
        MockWindowManagerBackend b;
        QSignalSpy spy(&b, &WindowManagerBackend::windowsChanged);

        QList<WindowInfo> list;
        WindowInfo w;
        w.windowId  = QStringLiteral("wl-100");
        w.title     = QStringLiteral("Test Window");
        w.appId     = QStringLiteral("org.example.app");
        w.isActive  = true;
        w.isMinimized = false;
        list.append(w);
        b.setWindows(list);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(b.windows().size(), 1);
        QCOMPARE(b.windows().first().windowId, QStringLiteral("wl-100"));
        QCOMPARE(b.windows().first().title,    QStringLiteral("Test Window"));
        QCOMPARE(b.windows().first().appId,    QStringLiteral("org.example.app"));
        QVERIFY(b.windows().first().isActive);
        QVERIFY(!b.windows().first().isMinimized);
    }

    void testSetWorkspaces()
    {
        MockWindowManagerBackend b;
        QSignalSpy wsspy(&b, &WindowManagerBackend::workspacesChanged);
        QSignalSpy cwspy(&b, &WindowManagerBackend::currentWorkspaceIndexChanged);

        QList<WorkspaceInfo> wsList;
        for (int i = 0; i < 4; ++i) {
            WorkspaceInfo ws;
            ws.workspaceIndex = i;
            ws.name           = QStringLiteral("Desktop %1").arg(i + 1);
            ws.active         = (i == 2);
            wsList.append(ws);
        }
        b.setWorkspaces(wsList, 2);

        QCOMPARE(wsspy.count(), 1);
        QCOMPARE(cwspy.count(), 1);
        QCOMPARE(b.workspaces().size(), 4);
        QCOMPARE(b.currentWorkspaceIndex(), 2);
        QVERIFY(b.workspaces().at(2).active);
        QVERIFY(!b.workspaces().at(0).active);
    }

    void testActivateWindow()
    {
        MockWindowManagerBackend b;
        b.activateWindow(QStringLiteral("wl-42"));
        QCOMPARE(b.activatedWindows.size(), 1);
        QCOMPARE(b.activatedWindows.first(), QStringLiteral("wl-42"));
    }

    void testCloseWindow()
    {
        MockWindowManagerBackend b;
        b.closeWindow(QStringLiteral("wl-99"));
        QCOMPARE(b.closedWindows.size(), 1);
        QCOMPARE(b.closedWindows.first(), QStringLiteral("wl-99"));
    }

    void testSwitchToWorkspace()
    {
        MockWindowManagerBackend b;
        QSignalSpy spy(&b, &WindowManagerBackend::currentWorkspaceIndexChanged);
        b.switchToWorkspace(3);
        QCOMPARE(b.switchedWorkspaces.size(), 1);
        QCOMPARE(b.switchedWorkspaces.first(), 3);
        QCOMPARE(b.currentWorkspaceIndex(), 3);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toInt(), 3);
    }

    void testWindowInfoFields()
    {
        WindowInfo w;
        // Verify new fields default-initialise correctly
        QVERIFY(w.windowId.isEmpty());
        QVERIFY(w.appId.isEmpty());
        QVERIFY(w.geometry.isNull());
        QVERIFY(!w.isActive);
        QVERIFY(!w.isMinimized);
        QCOMPARE(w.workspaceIndex, 0);
    }

    void testWorkspaceInfoFields()
    {
        WorkspaceInfo ws;
        QCOMPARE(ws.workspaceIndex, 0);
        QCOMPARE(ws.windowCount, 0);
        QVERIFY(!ws.active);
        QVERIFY(ws.name.isEmpty());
    }

    void testAvailabilitySignal()
    {
        MockWindowManagerBackend b;
        QSignalSpy spy(&b, &WindowManagerBackend::availabilityChanged);
        b.setAvailable(true);
        b.setAvailable(false);
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(0).first().toBool(), true);
        QCOMPARE(spy.at(1).first().toBool(), false);
    }
};

QTEST_GUILESS_MAIN(tst_MockBackend)
#include "tst_MockBackend.moc"
