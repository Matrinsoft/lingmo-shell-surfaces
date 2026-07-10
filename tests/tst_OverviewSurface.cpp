#include <QtTest/QtTest>
#include <LingmoShellSurfaces/OverviewSurface.h>
#include <LingmoShellSurfaces/Types.h>

using namespace Lingmo;

class tst_OverviewSurface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void defaultProperties()
    {
        OverviewSurface overview;
        QCOMPARE(overview.layer(), SurfaceLayer::Overlay);
        QVERIFY(!overview.isReady());
    }

    void layerIsAlwaysOverlay()
    {
        OverviewSurface overview;
        QCOMPARE(overview.layer(), SurfaceLayer::Overlay);
    }

    void readyStartsFalse()
    {
        OverviewSurface overview;
        QCOMPARE(overview.isReady(), false);
    }

    void windowModelNotNull()
    {
        OverviewSurface overview;
        QVERIFY(overview.windowModel() != nullptr);
    }

    void workspaceModelNotNull()
    {
        OverviewSurface overview;
        QVERIFY(overview.workspaceModel() != nullptr);
    }

    void windowModelHasExpectedRoles()
    {
        OverviewSurface overview;
        QAbstractItemModel *model = overview.windowModel();
        QVERIFY(model != nullptr);
        auto roleNames = model->roleNames();
        // Expect at least title, windowId, and appId roles
        QList<QByteArray> names = roleNames.values();
        QVERIFY(names.contains("title")    || names.contains("Title"));
        QVERIFY(names.contains("windowId") || names.contains("WindowId") || names.contains("wid"));
    }

    void workspaceModelHasExpectedRoles()
    {
        OverviewSurface overview;
        QAbstractItemModel *model = overview.workspaceModel();
        QVERIFY(model != nullptr);
        auto roleNames = model->roleNames();
        QList<QByteArray> names = roleNames.values();
        QVERIFY(names.contains("name") || names.contains("Name"));
    }
};

QTEST_MAIN(tst_OverviewSurface)
#include "tst_OverviewSurface.moc"
