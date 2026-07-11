#include "VirtualDesktopIntegration.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QVariant>

Q_LOGGING_CATEGORY(lcVDesktop, "lingmo.shell.surfaces.vdesktop")

// org.kde.KWin.VirtualDesktopManager property/signal types

// The desktops property returns aa{sv} — an array of dicts.
// Each dict contains: "id" (QString), "name" (QString), "position" (uint).
using DesktopDataList = QList<QVariantMap>;
Q_DECLARE_METATYPE(DesktopDataList)

namespace Lingmo {

VirtualDesktopIntegration::VirtualDesktopIntegration(QObject *parent)
    : QObject(parent)
{
}

VirtualDesktopIntegration::~VirtualDesktopIntegration() = default;

void VirtualDesktopIntegration::initialize()
{
    iface = new QDBusInterface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/VirtualDesktopManager"),
        QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
        QDBusConnection::sessionBus(),
        this);

    if (!iface->isValid()) {
        qCWarning(lcVDesktop) << "VirtualDesktopManager D-Bus not available:"
                              << iface->lastError().message();
        return;
    }

    // Subscribe to signals
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/VirtualDesktopManager"),
        QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
        QStringLiteral("currentChanged"),
        this,
        SLOT(onCurrentDesktopChanged(QString)));

    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/VirtualDesktopManager"),
        QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
        QStringLiteral("desktopAdded"),
        this,
        SLOT(onDesktopAdded(QString, uint)));

    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/VirtualDesktopManager"),
        QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
        QStringLiteral("desktopRemoved"),
        this,
        SLOT(onDesktopRemoved(QString)));

    refresh();
    Q_EMIT ready();
}

bool VirtualDesktopIntegration::isValid() const
{
    return iface && iface->isValid();
}

int VirtualDesktopIntegration::currentIndex() const
{
    for (int i = 0; i < desktopList.size(); ++i) {
        if (desktopList.at(i).id == currentDesktopId)
            return i;
    }
    return 0;
}

// ── Private refresh ───────────────────────────────────────────────────────────

void VirtualDesktopIntegration::refresh()
{
    if (!iface || !iface->isValid())
        return;

    desktopList.clear();

    // Read the "desktops" property — returns aa{sv}
    const QVariant desktopsProp =
        iface->property("desktops");

    if (!desktopsProp.isValid()) {
        qCWarning(lcVDesktop) << "Failed to read 'desktops' property:"
                              << iface->lastError().message();
        // Fallback: build a single desktop from currentDesktop.
        const QVariant currentProp = iface->property("current");
        currentDesktopId = currentProp.isValid()
                           ? currentProp.toString()
                           : QStringLiteral("0");

        DesktopInfo d;
        d.id     = currentDesktopId;
        d.name   = QStringLiteral("Desktop 1");
        d.active = true;
        desktopList.append(d);
        Q_EMIT listChanged();
        return;
    }

    // D-Bus delivers aa{sv} as a QList<QVariant> where each element
    // is a QVariantMap.
    const QVariant currentProp = iface->property("current");
    currentDesktopId = currentProp.isValid()
                       ? currentProp.toString()
                       : QString();

    const QDBusArgument arg = desktopsProp.value<QDBusArgument>();
    if (!arg.currentSignature().isEmpty()) {
        arg.beginArray();
        while (!arg.atEnd()) {
            QVariantMap map;
            arg >> map;
            DesktopInfo d;
            d.id     = map.value(QStringLiteral("id")).toString();
            d.name   = map.value(QStringLiteral("name")).toString();
            d.active = (d.id == currentDesktopId);
            if (!d.id.isEmpty())
                desktopList.append(d);
        }
        arg.endArray();
    }

    // If we still got nothing, make a single placeholder.
    if (desktopList.isEmpty()) {
        DesktopInfo d;
        d.id     = currentDesktopId.isEmpty() ? QStringLiteral("0") : currentDesktopId;
        d.name   = QStringLiteral("Desktop 1");
        d.active = true;
        desktopList.append(d);
    }

    qCDebug(lcVDesktop) << "Refreshed" << desktopList.size() << "desktops; current ="
                        << currentDesktopId;
    Q_EMIT listChanged();
}

// ── D-Bus signal slots ────────────────────────────────────────────────────────

void VirtualDesktopIntegration::onCurrentDesktopChanged(const QString &id)
{
    if (id == currentDesktopId)
        return;
    currentDesktopId = id;
    // Update active flag on each desktop.
    for (auto &d : desktopList)
        d.active = (d.id == id);

    const int idx = currentIndex();
    qCDebug(lcVDesktop) << "Current desktop changed to" << id << "(index" << idx << ")";
    Q_EMIT currentChanged(idx);
}

void VirtualDesktopIntegration::onDesktopAdded(const QString &/*id*/, uint /*position*/)
{
    refresh();
    Q_EMIT listChanged();
}

void VirtualDesktopIntegration::onDesktopRemoved(const QString &/*id*/)
{
    refresh();
    Q_EMIT listChanged();
}

} // namespace Lingmo
