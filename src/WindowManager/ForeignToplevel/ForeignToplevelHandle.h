#pragma once
#include <QObject>
#include <QRect>
#include <QSet>
#include <QString>

#include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

namespace Lingmo {

// Wraps a single zwlr_foreign_toplevel_handle_v1.
// Accumulates title/app_id/state events and emits ready() after the
// first done event, then changed() on each subsequent done.
class ForeignToplevelHandle
    : public QObject
    , public QtWayland::zwlr_foreign_toplevel_handle_v1
{
    Q_OBJECT
public:
    explicit ForeignToplevelHandle(struct ::zwlr_foreign_toplevel_handle_v1 *handle,
                                    QObject *parent = nullptr);
    ~ForeignToplevelHandle() override;

    [[nodiscard]] QString handleId()    const { return id; }
    [[nodiscard]] QString title()       const { return currentTitle; }
    [[nodiscard]] QString appId()       const { return currentAppId; }
    [[nodiscard]] bool    isMinimized() const { return states.contains(State::minimized); }
    [[nodiscard]] bool    isActive()    const { return states.contains(State::activated); }
    [[nodiscard]] bool    isMaximized() const { return states.contains(State::maximized); }

    // Asks the compositor to activate this toplevel on the default seat.
    // Seat argument required by protocol; we pass nullptr and rely on
    // the compositor's default behaviour.
    void requestActivate();

    // Asks the compositor to close this toplevel.
    void requestClose();

Q_SIGNALS:
    void ready();    // first done — handle is fully initialised
    void changed();  // subsequent done — some property updated
    void closed();   // compositor destroyed the handle

private:
    enum class State { maximized = 0, minimized = 1, activated = 2, fullscreen = 3 };

    // QtWayland event overrides
    void zwlr_foreign_toplevel_handle_v1_title(const QString &title)    override;
    void zwlr_foreign_toplevel_handle_v1_app_id(const QString &appId)   override;
    void zwlr_foreign_toplevel_handle_v1_state(wl_array *stateArray)    override;
    void zwlr_foreign_toplevel_handle_v1_done()                         override;
    void zwlr_foreign_toplevel_handle_v1_closed()                       override;

    QString   id;
    QString   currentTitle;
    QString   currentAppId;
    QSet<State> states;
    bool      initialised = false;
};

} // namespace Lingmo
