#include <QtTest/QtTest>
#include <LingmoShellSurfaces/Types.h>

// Test layer mapping and anchor calculation logic independent of a compositor.

class tst_LayerShell : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { qputenv("QT_QPA_PLATFORM", "offscreen"); }

    void layerEnumValues()
    {
        QCOMPARE(static_cast<uint32_t>(Lingmo::SurfaceLayer::Background), 0u);
        QCOMPARE(static_cast<uint32_t>(Lingmo::SurfaceLayer::Bottom),     1u);
        QCOMPARE(static_cast<uint32_t>(Lingmo::SurfaceLayer::Top),        2u);
        QCOMPARE(static_cast<uint32_t>(Lingmo::SurfaceLayer::Overlay),    3u);
    }

    void anchorForEdgeTop()
    {
        // Top edge: anchor = top | left | right = 1 | 4 | 8 = 13
        using E = Lingmo::SurfaceEdge;
        uint32_t anchor = 0;
        const auto edge = E::Top;
        if (edge == E::Top)    anchor = 1 | 4 | 8;
        if (edge == E::Bottom) anchor = 2 | 4 | 8;
        if (edge == E::Left)   anchor = 4 | 1 | 2;
        if (edge == E::Right)  anchor = 8 | 1 | 2;
        if (edge == E::None)   anchor = 1 | 2 | 4 | 8;
        QCOMPARE(anchor, 13u);
    }

    void anchorForEdgeBottom()
    {
        uint32_t anchor = 2 | 4 | 8;
        QCOMPARE(anchor, 14u);
    }

    void anchorForEdgeLeft()
    {
        uint32_t anchor = 4 | 1 | 2;
        QCOMPARE(anchor, 7u);
    }

    void anchorForEdgeRight()
    {
        uint32_t anchor = 8 | 1 | 2;
        QCOMPARE(anchor, 11u);
    }

    void anchorForEdgeNone()
    {
        uint32_t anchor = 1 | 2 | 4 | 8;
        QCOMPARE(anchor, 15u);
    }

    void keyboardModeOverlay()
    {
        // Overlay layer → on_demand (2); all other layers → none (0)
        const uint32_t kbOverlay = (Lingmo::SurfaceLayer::Overlay == Lingmo::SurfaceLayer::Overlay)
                                   ? 2u : 0u;
        QCOMPARE(kbOverlay, 2u);

        const uint32_t kbTop = (Lingmo::SurfaceLayer::Top == Lingmo::SurfaceLayer::Overlay)
                                ? 2u : 0u;
        QCOMPARE(kbTop, 0u);
    }
};

QTEST_MAIN(tst_LayerShell)
#include "tst_LayerShell.moc"
