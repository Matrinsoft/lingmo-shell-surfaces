#pragma once

#include <LingmoShellSurfaces/PanelSurface.h>
#include "ShellSurface_p.h"

#include <QList>
#include <QPointer>

namespace Lingmo {

class PanelSurfacePrivate : public ShellSurfacePrivate
{
public:
    explicit PanelSurfacePrivate(PanelSurface *q);

    SurfaceEdge edge = SurfaceEdge::Top;
    int height = 36;
    bool autohide = false;
    int exclusiveZone = 36;

    QList<QPointer<QObject>> applets;

    void updateExclusiveZone();
    void applyEdgeToWindow();
};

} // namespace Lingmo
