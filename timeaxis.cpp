#include "timeaxis.h"

const msecs TimeInfo::msecsInDay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(24));
const msecs TimeInfo::msecsIn3hours = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(3));
const msecs TimeInfo::msecsInhour = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(1));


//AxisInfo TimeAxis::getAi() const
//{
//    return m_ai;
//}

//void TimeAxis::setAi(const AxisInfo &ai)
//{
//    m_ai = ai;
//}

float TimeAxis::getZoomOffsetMsecs() const
{
    return m_zoomOffsetMsecs;
}

void TimeAxis::setZoomOffsetMsecs(float zoomOffsetMsecs)
{
    m_zoomOffsetMsecs = zoomOffsetMsecs;
}

float TimeAxis::getHourWidthInPx() const
{
    return m_hourWidthInPx;
}


void TimeAxis::setHourWidthInPx(float hourWidthInPx)
{
    m_hourWidthInPx = hourWidthInPx;
}

float TimeAxis::getDayWidthInPx() const
{
    return m_dayWidthInPx;
}

void TimeAxis::setDayWidthInPx(float dayWidthInPx)
{
    m_dayWidthInPx = dayWidthInPx;
}

float TimeAxis::getStepInPx() const
{
    return m_stepInPx;
}

void TimeAxis::setStepInPx(float stepInPx)
{
    m_stepInPx = stepInPx;
}

float TimeAxis::getDragOffsetPx() const
{
    return m_dragOffsetPx;
}

void TimeAxis::setDragOffsetPx(float dragOffsetPx)
{
    m_dragOffsetPx = dragOffsetPx;
}

float TimeAxis::getDragOffsetCurPx() const
{
    return m_dragOffsetCurPx;
}

void TimeAxis::setDragOffsetCurPx(float dragOffsetCurPx)
{
    m_dragOffsetCurPx = dragOffsetCurPx;
}

