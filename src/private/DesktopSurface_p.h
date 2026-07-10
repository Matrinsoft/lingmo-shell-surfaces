#pragma once

#include <LingmoShellSurfaces/DesktopSurface.h>
#include "ShellSurface_p.h"

#include <QColor>
#include <QString>

namespace Lingmo {

class DesktopSurfacePrivate : public ShellSurfacePrivate
{
public:
    explicit DesktopSurfacePrivate(DesktopSurface *q);

    QString wallpaperSource;
    QColor wallpaperColor = QColor(QStringLiteral("#1a1a2e"));
    DesktopSurface::WallpaperFillMode fillMode = DesktopSurface::WallpaperFillMode::PreserveAspectCrop;
};

} // namespace Lingmo
