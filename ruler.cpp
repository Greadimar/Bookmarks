#include "ruler.h"


int RenderInfo::dragOffset() const
{
    return _dragOffset;
}

void RenderInfo::setDragOffset(int dragOffset)
{
    _dragOffset = dragOffset;
}

float RenderInfo::zoomRatio() const
{
    return _zoomRatio;
}

void RenderInfo::setZoomRatio(float zoomRatio)
{
    _zoomRatio = zoomRatio;
}
