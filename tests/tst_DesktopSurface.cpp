#include <QtTest/QtTest>
#include <LingmoShellSurfaces/DesktopSurface.h>
#include <QColor>
#include <LingmoShellSurfaces/Types.h>

using namespace Lingmo;

class tst_DesktopSurface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void defaultProperties()
    {
        DesktopSurface desktop;
        QVERIFY(desktop.wallpaperSource().isEmpty());
        QCOMPARE(desktop.wallpaperColor(), QColor("#1a1a2e"));
        QCOMPARE(desktop.layer(), SurfaceLayer::Background);
    }

    void setWallpaperSource()
    {
        DesktopSurface desktop;
        QSignalSpy spy(&desktop, &DesktopSurface::wallpaperSourceChanged);
        const QString path = QStringLiteral("file:///usr/share/lingmo/wallpapers/default.jpg");
        desktop.setWallpaperSource(path);
        QCOMPARE(desktop.wallpaperSource(), path);
        QCOMPARE(spy.count(), 1);
    }

    void setWallpaperSourceSameValueNoSignal()
    {
        DesktopSurface desktop;
        QSignalSpy spy(&desktop, &DesktopSurface::wallpaperSourceChanged);
        desktop.setWallpaperSource(QString()); // same as default
        QCOMPARE(spy.count(), 0);
    }

    void setWallpaperColor()
    {
        DesktopSurface desktop;
        QSignalSpy spy(&desktop, &DesktopSurface::wallpaperColorChanged);
        desktop.setWallpaperColor(QColor(Qt::black));
        QCOMPARE(desktop.wallpaperColor(), QColor(Qt::black));
        QCOMPARE(spy.count(), 1);
    }

    void setFillMode()
    {
        DesktopSurface desktop;
        QSignalSpy spy(&desktop, &DesktopSurface::fillModeChanged);
        desktop.setFillMode(DesktopSurface::Stretch);
        QCOMPARE(desktop.fillMode(), DesktopSurface::Stretch);
        QCOMPARE(spy.count(), 1);
    }

    void layerIsAlwaysBackground()
    {
        DesktopSurface desktop;
        QCOMPARE(desktop.layer(), SurfaceLayer::Background);
    }

    void fillModeDefaultIsPreserveAspectCrop()
    {
        DesktopSurface desktop;
        QCOMPARE(desktop.fillMode(), DesktopSurface::PreserveAspectCrop);
    }
};

QTEST_MAIN(tst_DesktopSurface)
#include "tst_DesktopSurface.moc"
