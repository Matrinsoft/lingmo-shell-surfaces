#pragma once

#include <LingmoShellSurfaces/LingmoShellSurfacesExport.h>
#include <LingmoShellSurfaces/ShellSurface.h>
#include <LingmoShellSurfaces/Types.h>

#include <QColor>
#include <QPoint>
#include <QObject>
#include <QString>

namespace Lingmo {

class DesktopSurfacePrivate;

// DesktopSurface — fullscreen desktop background surface.
//
// Occupies the Background layer on Wayland, filling the entire screen below
// all windows.  On X11 it is a full-screen override-redirect window set as
// the desktop window (_NET_WM_WINDOW_TYPE_DESKTOP).
//
// Responsibilities:
//  - Renders the wallpaper (image or solid colour).
//  - Provides a container for desktop icon items (supplied by the file
//    manager via a plugin — not implemented here).
//  - Emits contextMenuRequested() so a shell plugin can show a context menu.
//
// Example (QML):
//   DesktopSurface {
//       wallpaperSource: "file:///usr/share/lingmo/wallpapers/default.jpg"
//   }
class LINGMOSHELLSURFACES_EXPORT DesktopSurface : public ShellSurface
{
    Q_OBJECT

    Q_PROPERTY(QString wallpaperSource
               READ wallpaperSource WRITE setWallpaperSource
               NOTIFY wallpaperSourceChanged)
    Q_PROPERTY(QColor wallpaperColor
               READ wallpaperColor WRITE setWallpaperColor
               NOTIFY wallpaperColorChanged)
    Q_PROPERTY(WallpaperFillMode fillMode
               READ fillMode WRITE setFillMode NOTIFY fillModeChanged)

public:
    enum class WallpaperFillMode : uint8_t {
        Stretch,       // scale ignoring aspect ratio
        PreserveAspectFit,   // letterbox
        PreserveAspectCrop,  // crop to fill
        Tile
    };
    Q_ENUM(WallpaperFillMode)

    explicit DesktopSurface(QObject *parent = nullptr);
    ~DesktopSurface() override;

    // URI of the wallpaper image (file:// or qrc://).
    // Empty string falls back to wallpaperColor.
    [[nodiscard]] QString wallpaperSource() const;
    void setWallpaperSource(const QString &source);

    // Solid background colour used when wallpaperSource is empty or fails to
    // load.  Format: "#RRGGBB" or any QColor-parseable string.
    [[nodiscard]] QColor wallpaperColor() const;
    void setWallpaperColor(const QColor &color);

    [[nodiscard]] WallpaperFillMode fillMode() const;
    void setFillMode(WallpaperFillMode mode);

    SurfaceLayer layer() const override;

Q_SIGNALS:
    void wallpaperSourceChanged(const QString &source);
    void wallpaperColorChanged(const QColor &color);
    void fillModeChanged(WallpaperFillMode mode);

    // Emitted when the user right-clicks the desktop background.
    // QPoint is in screen coordinates.
    void contextMenuRequested(QPoint globalPos);

protected:
    void createWindow() override;
};

} // namespace Lingmo
