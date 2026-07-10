#include <QtTest/QtTest>
#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

using namespace Lingmo;

class tst_ShellSurface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // Offscreen platform is set via QT_QPA_PLATFORM=offscreen
    }

    void defaultProperties()
    {
        // ShellSurface is abstract; we test via concrete subclass in other test
        // Here we test enums and types
        SurfaceGeometry geo;
        geo.x = 0; geo.y = 0; geo.width = 1920; geo.height = 32;
        QCOMPARE(geo.x, 0);
        QCOMPARE(geo.width, 1920);
    }

    void surfaceEdgeValues()
    {
        QCOMPARE(static_cast<int>(SurfaceEdge::Top),    0);
        QCOMPARE(static_cast<int>(SurfaceEdge::Bottom), 1);
        QCOMPARE(static_cast<int>(SurfaceEdge::Left),   2);
        QCOMPARE(static_cast<int>(SurfaceEdge::Right),  3);
        QCOMPARE(static_cast<int>(SurfaceEdge::None),   4);
    }

    void surfaceLayerValues()
    {
        QCOMPARE(static_cast<int>(SurfaceLayer::Background), 0);
        QCOMPARE(static_cast<int>(SurfaceLayer::Bottom),     1);
        QCOMPARE(static_cast<int>(SurfaceLayer::Top),        2);
        QCOMPARE(static_cast<int>(SurfaceLayer::Overlay),    3);
    }

    void surfacePlatformValues()
    {
        QVERIFY(static_cast<int>(SurfacePlatform::Unknown)   >= 0);
        QVERIFY(static_cast<int>(SurfacePlatform::Wayland)   >= 0);
        QVERIFY(static_cast<int>(SurfacePlatform::X11)       >= 0);
        QVERIFY(static_cast<int>(SurfacePlatform::Offscreen) >= 0);
    }

    void geometryDebugOutput()
    {
        SurfaceGeometry geo{10, 20, 1280, 720};
        QString s;
        QDebug dbg(&s);
        dbg << geo;
        QVERIFY(s.contains(QStringLiteral("1280")));
        QVERIFY(s.contains(QStringLiteral("720")));
    }
};

QTEST_MAIN(tst_ShellSurface)
#include "tst_ShellSurface.moc"
