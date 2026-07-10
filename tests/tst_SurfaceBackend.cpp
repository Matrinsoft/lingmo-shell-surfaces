#include <QtTest/QtTest>
#include <LingmoShellSurfaces/PanelSurface.h>
#include <LingmoShellSurfaces/DesktopSurface.h>
#include <LingmoShellSurfaces/OverviewSurface.h>
#include <LingmoShellSurfaces/Types.h>

using namespace Lingmo;

/**
 * tst_SurfaceBackend — tests that the SurfaceBackend factory returns a
 * valid non-null backend for each supported surface type in an offscreen
 * environment, and that the surfaces can be shown/hidden without crashing.
 */
class tst_SurfaceBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void panelBackendCreatedOnShow()
    {
        PanelSurface panel;
        // Should not crash; on offscreen platform uses GenericSurfaceBackend
        QVERIFY(!panel.isVisible());
        panel.show();
        QVERIFY(panel.isVisible());
        panel.hide();
        QVERIFY(!panel.isVisible());
    }

    void desktopBackendCreatedOnShow()
    {
        DesktopSurface desktop;
        QVERIFY(!desktop.isVisible());
        desktop.show();
        QVERIFY(desktop.isVisible());
        desktop.hide();
        QVERIFY(!desktop.isVisible());
    }

    void overviewBackendCreatedOnShow()
    {
        OverviewSurface overview;
        QVERIFY(!overview.isVisible());
        overview.show();
        QVERIFY(overview.isVisible());
        overview.hide();
        QVERIFY(!overview.isVisible());
    }

    void platformDetectedAsOffscreen()
    {
        PanelSurface panel;
        panel.show();
        QCOMPARE(panel.platform(), SurfacePlatform::Offscreen);
        panel.hide();
    }

    void reloadDoesNotCrash()
    {
        PanelSurface panel;
        panel.show();
        panel.reload(); // should recreate window without crash
        QVERIFY(panel.isVisible());
        panel.hide();
    }

    void multipleShowCallsIdempotent()
    {
        PanelSurface panel;
        panel.show();
        panel.show(); // second call should be no-op
        QVERIFY(panel.isVisible());
        panel.hide();
    }

    void multipleHideCallsIdempotent()
    {
        PanelSurface panel;
        panel.hide();
        panel.hide(); // should not crash
        QVERIFY(!panel.isVisible());
    }
};

QTEST_MAIN(tst_SurfaceBackend)
#include "tst_SurfaceBackend.moc"
