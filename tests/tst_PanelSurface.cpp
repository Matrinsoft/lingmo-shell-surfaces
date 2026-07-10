#include <QtTest/QtTest>
#include <LingmoShellSurfaces/PanelSurface.h>
#include <LingmoShellSurfaces/Types.h>

using namespace Lingmo;

class tst_PanelSurface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void defaultProperties()
    {
        PanelSurface panel;
        QCOMPARE(panel.edge(), SurfaceEdge::Top);
        QCOMPARE(panel.panelHeight(), 36);
        QCOMPARE(panel.autohide(), false);
        QCOMPARE(panel.exclusiveZone(), 36);
        QCOMPARE(panel.layer(), SurfaceLayer::Top);
    }

    void setEdge()
    {
        PanelSurface panel;
        QSignalSpy spy(&panel, &PanelSurface::edgeChanged);
        panel.setEdge(SurfaceEdge::Bottom);
        QCOMPARE(panel.edge(), SurfaceEdge::Bottom);
        QCOMPARE(spy.count(), 1);
    }

    void setEdgeSameValueNoSignal()
    {
        PanelSurface panel;
        QSignalSpy spy(&panel, &PanelSurface::edgeChanged);
        panel.setEdge(SurfaceEdge::Top); // same as default
        QCOMPARE(spy.count(), 0);
    }

    void setPanelHeight()
    {
        PanelSurface panel;
        QSignalSpy spy(&panel, &PanelSurface::panelHeightChanged);
        panel.setPanelHeight(48);
        QCOMPARE(panel.panelHeight(), 48);
        QCOMPARE(spy.count(), 1);
    }

    void setAutohide()
    {
        PanelSurface panel;
        QSignalSpy spy(&panel, &PanelSurface::autohideChanged);
        panel.setAutohide(true);
        QCOMPARE(panel.autohide(), true);
        QCOMPARE(spy.count(), 1);
    }

    void appletManagement()
    {
        PanelSurface panel;
        QCOMPARE(panel.appletCount(), 0);

        QObject applet1, applet2;
        panel.appendApplet(&applet1);
        QCOMPARE(panel.appletCount(), 1);

        panel.appendApplet(&applet2);
        QCOMPARE(panel.appletCount(), 2);

        panel.removeApplet(&applet1);
        QCOMPARE(panel.appletCount(), 1);
    }

    void exclusiveZoneUpdatesWithHeight()
    {
        PanelSurface panel;
        panel.setPanelHeight(48);
        QCOMPARE(panel.exclusiveZone(), 48);
    }

    void layerIsAlwaysTop()
    {
        PanelSurface panel;
        QCOMPARE(panel.layer(), SurfaceLayer::Top);
    }
};

QTEST_MAIN(tst_PanelSurface)
#include "tst_PanelSurface.moc"
