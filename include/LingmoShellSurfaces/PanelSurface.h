#pragma once

#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QObject>
#include <QString>

namespace Lingmo {

class PanelSurfacePrivate;

// PanelSurface — top or bottom panel anchored to a screen edge.
//
// The panel occupies a layer-shell Top surface on Wayland, reserving an
// exclusive zone so other windows are tiled below/above it.  On X11 it uses
// an override-redirect window with _NET_WM_STRUT extended hints.
//
// Applets are loaded via lingmo-plugin (org.lingmo.applet.*) and laid out
// inside the panel's QML item tree.  The panel itself provides only the
// container — no built-in applets are shipped here.
//
// Example (QML):
//   PanelSurface {
//       edge: SurfaceEdge.Top
//       height: 40
//   }
class LINGMOSHELLSURFACES_EXPORT PanelSurface : public ShellSurface
{
    Q_OBJECT

    Q_PROPERTY(Lingmo::SurfaceEdge edge
               READ edge WRITE setEdge NOTIFY edgeChanged)
    Q_PROPERTY(int panelHeight
               READ panelHeight WRITE setPanelHeight NOTIFY panelHeightChanged)
    Q_PROPERTY(bool autohide
               READ autohide WRITE setAutohide NOTIFY autohideChanged)
    Q_PROPERTY(int exclusiveZone
               READ exclusiveZone NOTIFY exclusiveZoneChanged)

public:
    explicit PanelSurface(QObject *parent = nullptr);
    ~PanelSurface() override;

    // The screen edge this panel is anchored to (default: Top).
    [[nodiscard]] SurfaceEdge edge() const;
    void setEdge(SurfaceEdge edge);

    // Pixel height of the panel (default: 40).
    [[nodiscard]] int panelHeight() const;
    void setPanelHeight(int height);

    // When true the panel slides off-screen when not hovered.
    [[nodiscard]] bool autohide() const;
    void setAutohide(bool autohide);

    // The exclusive zone currently reported to the compositor (read-only).
    // Equals panelHeight() unless autohide is active.
    [[nodiscard]] int exclusiveZone() const;

    // Appends a loaded applet QObject into the panel's applet model.
    // The panel takes ownership of the applet.
    void appendApplet(QObject *applet);

    // Removes the applet by pointer (takes precedence) or by index.
    void removeApplet(QObject *applet);
    void removeApplet(int index);

    // Returns the number of loaded applets.
    [[nodiscard]] int appletCount() const;

    SurfaceLayer layer() const override;

Q_SIGNALS:
    void edgeChanged(Lingmo::SurfaceEdge edge);
    void panelHeightChanged(int height);
    void autohideChanged(bool autohide);
    void exclusiveZoneChanged(int zone);

protected:
    void createWindow() override;

};

} // namespace Lingmo
