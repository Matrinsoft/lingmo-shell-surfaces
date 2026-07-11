#pragma once

#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <LingmoShellSurfaces/Types.h>

#include <QObject>
#include <QScreen>
#include <QString>

#include <memory>

class QWindow;

namespace Lingmo {

class ShellSurfacePrivate;

// ShellSurface — abstract base for all Lingmo shell surfaces.
//
// Concrete subclasses (PanelSurface, DesktopSurface, OverviewSurface) extend
// this with surface-type-specific behaviour.  Each surface owns one QWindow
// created by the concrete implementation.
//
// Lifecycle:
//   1. Construct the subclass.
//   2. Assign to a screen (optional; defaults to primary screen).
//   3. Call show() to map the surface.
//   4. Call hide() to unmap without destroying.
//   5. Destroy the object to release all resources.
//
// Thread-safety: All methods MUST be called from the main thread.
class LINGMOSHELLSURFACES_EXPORT ShellSurface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ isVisible NOTIFY visibilityChanged)
    Q_PROPERTY(QScreen *screen READ screen WRITE setScreen NOTIFY screenChanged)
    Q_PROPERTY(Lingmo::SurfaceLayer layer READ layer CONSTANT)
    Q_PROPERTY(Lingmo::SurfacePlatform platform READ platform CONSTANT)

public:
    ~ShellSurface() override;

    // Returns true if the surface is currently mapped (visible).
    [[nodiscard]] bool isVisible() const;

    // Returns the screen this surface is attached to.
    // Never null after construction (defaults to primary screen).
    [[nodiscard]] QScreen *screen() const;

    // Moves the surface to the given screen.  Calls reload() internally.
    void setScreen(QScreen *screen);

    // Returns the Wayland layer this surface occupies (constant per type).
    [[nodiscard]] virtual SurfaceLayer layer() const = 0;

    // Returns the detected platform (Wayland / X11 / Offscreen).
    [[nodiscard]] SurfacePlatform platform() const;

    // Returns the native QWindow backing this surface, or nullptr before
    // show() has been called for the first time.
    [[nodiscard]] QWindow *window() const;

public Q_SLOTS:
    // Maps the surface on the assigned screen.
    // If the surface is already visible this is a no-op.
    virtual void show();

    // Unmaps the surface without destroying it.
    virtual void hide();

    // Destroys the native surface and recreates it, re-reading configuration.
    // Use when the screen or DPI changes.
    virtual void reload();

Q_SIGNALS:
    void visibilityChanged(bool visible);
    void screenChanged(QScreen *screen);

protected:
    explicit ShellSurface(ShellSurfacePrivate *d, QObject *parent = nullptr);

    // Called by show()/reload() to create the platform-specific window.
    // Subclasses implement this to configure their QWindow.
    virtual void createWindow() = 0;

    // Called by hide() / destructor to destroy the platform window.
    virtual void destroyWindow();

    std::unique_ptr<ShellSurfacePrivate> d;
};

} // namespace Lingmo
