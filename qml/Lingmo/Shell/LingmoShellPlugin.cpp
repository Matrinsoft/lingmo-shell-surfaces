#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include <LingmoShellSurfaces/PanelSurface.h>
#include <LingmoShellSurfaces/DesktopSurface.h>
#include <LingmoShellSurfaces/OverviewSurface.h>
#include <LingmoShellSurfaces/Types.h>

class LingmoShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Lingmo.Shell"));

        // Abstract base — exposes properties visible/screen/layer/platform to QML.
        qmlRegisterUncreatableType<Lingmo::ShellSurface>(
            uri, 1, 0, "ShellSurface",
            QStringLiteral("ShellSurface is abstract"));

        // Concrete surface items.
        qmlRegisterType<Lingmo::PanelSurface>(uri, 1, 0, "PanelSurfaceItem");
        qmlRegisterType<Lingmo::DesktopSurface>(uri, 1, 0, "DesktopSurfaceItem");
        qmlRegisterType<Lingmo::OverviewSurface>(uri, 1, 0, "OverviewSurfaceItem");

        // Namespace enums — Q_NAMESPACE + Q_ENUM_NS gives staticMetaObject.
        // Access in QML: LingmoShellSurfaces.Top, LingmoShellSurfaces.Background, …
        qmlRegisterUncreatableMetaObject(
            Lingmo::staticMetaObject,
            uri, 1, 0, "LingmoShellSurfaces",
            QStringLiteral("Access to enums only"));
    }
};

#include "LingmoShellPlugin.moc"
