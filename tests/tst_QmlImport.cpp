#include <QtTest/QtTest>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QUrl>

/**
 * tst_QmlImport — smoke-tests that the Lingmo.Shell QML plugin loads
 * and its types are accessible from QML.
 */
class tst_QmlImport : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");

#ifdef QML_IMPORT_PATH
        m_engine.addImportPath(QStringLiteral(QML_IMPORT_PATH));
#endif
    }

    void panelSurfaceCreatable()
    {
        QQmlComponent component(&m_engine);
        component.setData(R"(
            import Lingmo.Shell 1.0
            PanelSurfaceItem { }
        )", QUrl());
        QVERIFY2(component.status() == QQmlComponent::Ready,
                 qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }

    void desktopSurfaceCreatable()
    {
        QQmlComponent component(&m_engine);
        component.setData(R"(
            import Lingmo.Shell 1.0
            DesktopSurfaceItem { }
        )", QUrl());
        QVERIFY2(component.status() == QQmlComponent::Ready,
                 qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }

    void overviewSurfaceCreatable()
    {
        QQmlComponent component(&m_engine);
        component.setData(R"(
            import Lingmo.Shell 1.0
            OverviewSurfaceItem { }
        )", QUrl());
        QVERIFY2(component.status() == QQmlComponent::Ready,
                 qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }

    void enumsAccessibleFromQml()
    {
        QQmlComponent component(&m_engine);
        component.setData(R"(
            import QtQuick 2.15
            import Lingmo.Shell 1.0
            Item {
                property int topLayer: LingmoShellSurfaces.Top
            }
        )", QUrl());
        QVERIFY2(component.status() == QQmlComponent::Ready,
                 qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }

private:
    QQmlEngine m_engine;
};

QTEST_MAIN(tst_QmlImport)
#include "tst_QmlImport.moc"
