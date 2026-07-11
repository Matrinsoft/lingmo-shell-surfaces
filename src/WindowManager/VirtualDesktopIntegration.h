#pragma once
#include <QDBusInterface>
#include <QObject>
#include <QString>
#include <QStringList>

namespace Lingmo {

struct DesktopInfo {
    QString id;          // UUID string from VirtualDesktopManager
    QString name;
    bool    active = false;
};

// Wraps org.kde.KWin.VirtualDesktopManager D-Bus interface.
// This service lives on the session bus at /VirtualDesktopManager and
// exposes the desktop list, count, and the current desktop UUID.
class VirtualDesktopIntegration : public QObject
{
    Q_OBJECT
public:
    explicit VirtualDesktopIntegration(QObject *parent = nullptr);
    ~VirtualDesktopIntegration() override;

    // Must be called after construction.  Connects to D-Bus and subscribes
    // to change signals.  Emits ready() when the first refresh completes.
    void initialize();

    [[nodiscard]] bool               isValid()        const;
    [[nodiscard]] QList<DesktopInfo> desktops()       const { return desktopList; }
    [[nodiscard]] int                count()          const { return desktopList.size(); }
    [[nodiscard]] QString            currentId()      const { return currentDesktopId; }
    [[nodiscard]] int                currentIndex()   const;

Q_SIGNALS:
    void ready();
    void currentChanged(int newIndex);
    void listChanged();

private Q_SLOTS:
    void onCurrentDesktopChanged(const QString &id);
    void onDesktopAdded(const QString &id, uint position);
    void onDesktopRemoved(const QString &id);

private:
    void refresh();

    QDBusInterface *iface = nullptr;
    QList<DesktopInfo> desktopList;
    QString currentDesktopId;
};

} // namespace Lingmo
