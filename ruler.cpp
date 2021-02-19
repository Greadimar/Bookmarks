#include "ruler.h"
#include <QParallelAnimationGroup>

Ruler::Ruler(const Palette &plt, RenderInfo &ri, const QPointer<TimeAxis> &c): m_plt(plt), m_ri(ri), m_ta(c){
    this->font.setStyleStrategy(QFont::ForceOutline);
    QFontMetrics fm(font);

    hour1xLabelsOffsetToCenter = fm.horizontalAdvance("1h")/2;
    hour2xLabelsOffsetToCenter = fm.horizontalAdvance("24h")/2;
    hourLabelsBottomPos = linesHeight + fm.height() + hourLabelsTopMargin;
    ri.rulerBottomY = hourLabelsBottomPos;
    ri.bookmarksTopY = ri.rulerBottomY + ri.bookmarksTopMargin;
    ri.bookmarksBottomY = ri.bookmarksTopY + ri.bookmarksHeight;

    setAcceptHoverEvents(true);

    //set initial params
    QTimer::singleShot(10, this, [=](){
        int viewWidth = scene()->views().first()->width();
        int rulerWidth = viewWidth;// - m_ri.leftMargin - m_ri.rightMargin;
        m_ta->rulerWidth.store(rulerWidth);
        auto hourw = static_cast<float>(rulerWidth/24);
        auto dayw = static_cast<float>(rulerWidth);

        //AxisInfo ai{hourw, dayw, hourw, 0, 0};
        m_ta->setHourWidthInPx(hourw);
        m_ta->setDayWidthInPx(dayw);
        m_ta->setStepInPx(hourw);
        m_ta->setDragOffsetPx(0);
        m_ta->setDragOffsetCurPx(0);
    });

    //init animation
     aniDayWidth = makeAnimation(m_ta, "dayWidth");
     aniHourWidth = makeAnimation(m_ta, "hourWidth");
     aniStepWidth = makeAnimation(m_ta, "step");
     aniZoomOffset = makeAnimation(m_ta, "zoomOffMsecs");
     aniMin = makeAnimation(m_ta, "min");
     aniMax = makeAnimation(m_ta, "max");
    // aniAxisInfo = makeAnimation(m_ta, "axisInfo");


     aniParGroup = new QParallelAnimationGroup(this);
     aniParGroup->addAnimation(aniZoomOffset);
     aniParGroup->addAnimation(aniMin);
     aniParGroup->addAnimation(aniMax);
     aniParGroup->addAnimation(aniDayWidth);
     aniParGroup->addAnimation(aniStepWidth);
     aniParGroup->addAnimation(aniHourWidth);

     aniZoomOffset->setDuration(animationDuration);
     aniMin->setDuration(animationDuration);
     aniMax->setDuration(animationDuration);
     aniDayWidth->setDuration(animationDuration);
     aniHourWidth->setDuration(animationDuration);
     aniZoomOffset->setDuration(animationDuration);
     aniStepWidth->setDuration(animationDuration);

     connect(aniParGroup, &QParallelAnimationGroup::finished, this, [=](){
        qDebug() << "after ani min max" << m_ta->getMin() << m_ta->getMax();
        qDebug() << "after ani zoom" << m_ta->getZoomOffsetMsecs();
        qDebug() << "after ani step drag" << m_ta->getStepInPx() << m_ta->getDragOffsetPx();
        qDebug() << "after ani day hour" << m_ta->getDayWidthInPx() << m_ta->getHourWidthInPx();
     });
}

void Ruler::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
    //  p->save();
    QPointF curPos = scenePos();
    int viewWidth = scene()->views().first()->width();
    int rulerWidth = viewWidth;
    float resizeSinceLastRender = rulerWidth / static_cast<float>(m_ta->rulerWidth);
    m_ta->setStepInPx(m_ta->getStepInPx() * resizeSinceLastRender);
    m_ta->setHourWidthInPx(m_ta->getHourWidthInPx() * resizeSinceLastRender);
    m_ta->setDayWidthInPx(m_ta->getHourWidthInPx() * 24);
    m_ta->rulerWidth.store(rulerWidth);

    //drawing ruler
    //    p->setPen(m_plt.rulerBackground);
    //    p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
    /*p->setPen(QColor{"#ed9121"});
    p->drawRect(0, 0, rulerWidth,  hourLabelsBottomPos);*/

    //drawing lines and labels
    p->setPen(QPen(m_plt.labels, 2));
    p->setFont(font);

    int time{0};
    int min{m_ta->getMin()};
    int max{m_ta->getMax()};

    auto rx = curPos.rx();

    static const int msecIn10h = (std::chrono::duration_cast<msecs>(std::chrono::hours(10))).count();
    static const int msecIn24h = (std::chrono::duration_cast<msecs>(std::chrono::hours(24))).count();
    int h = 0;
    for (; time < msecIn10h; time += m_ta->step){
        if (time >= min && time <= max){
            int curTimePx = m_ta->pxPosFromMsec(time);
            p->drawLine({rx + curTimePx, curPos.ry()}, QPointF{rx + curTimePx, curPos.ry() + linesHeight});
            p->drawText(QPointF{curTimePx - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(h));
        }
        h++;

    }
    for (; time <= msecIn24h; time += m_ta->step){
        if (time >= min && time <= max){
            int curTimePx = m_ta->pxPosFromMsec(time);
            p->drawLine({rx + curTimePx, curPos.ry()}, QPointF{rx + curTimePx, curPos.ry() + linesHeight});
            p->drawText(QPointF{curTimePx - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(h));
        }
        h++;
    }
    // p->restore();

    m_ri.lastRender = std::chrono::system_clock::now();

}

QPropertyAnimation *Ruler::makeAnimation(QObject *obj, const QString &propValue){
    QPropertyAnimation* anim = new QPropertyAnimation(obj, propValue.toUtf8(), this);
    return anim;
}


void Ruler::zoomToCenter(float targetZoomRatio){
    //   m_ta->zoom//Ratio = targetZoomRatio;

    int curPosInMsecs = m_ta->getMin() + m_ta->msecFromPx(curMouseXPos);
    auto targetDayWidth = m_ta->getDayWidthInPx() * targetZoomRatio;
    auto targetHourWidth = targetDayWidth/24;
    auto targetStepInPx = m_ta->getStepInPx() * targetZoomRatio;
    float targetMousePosMsec = m_ta->msecFromPx(curMouseXPos, targetHourWidth);
    auto targetZoomOffset = curPosInMsecs - targetMousePosMsec;
    int targetMin = targetZoomOffset ;
    int targetMax = targetMin + m_ta->msecFromPx(m_ta->rulerWidth, targetHourWidth);
    int targetDragOffset{0};





    switch (animation){
    case Animation::noAnimation:{
        m_ta->setDayWidthInPx(targetDayWidth);
        m_ta->setHourWidthInPx(targetHourWidth);
        m_ta->setDragOffsetPx(targetDragOffset);
        m_ta->setStepInPx(targetStepInPx);
        m_ta->setZoomOffsetMsecs(targetZoomOffset);
        m_ta->setMin(targetMin);
        m_ta->setMax(targetMax);
    }
        break;
    default:{
        aniMax->setStartValue(m_ta->getMax());
        aniMax->setEndValue(targetMax);
        aniMin->setStartValue(m_ta->getMin());
        aniMin->setEndValue(targetMin);
        aniZoomOffset->setStartValue(m_ta->getZoomOffsetMsecs());
        aniZoomOffset->setEndValue(targetZoomOffset);
        aniDayWidth->setStartValue(m_ta->getDayWidthInPx());
        aniDayWidth->setEndValue(targetDayWidth);
        aniHourWidth->setStartValue(m_ta->getHourWidthInPx());
        aniHourWidth->setEndValue(targetHourWidth);
        aniStepWidth->setStartValue(m_ta->getDayWidthInPx());
        aniStepWidth->setEndValue(targetStepInPx);

        //m_ta->ai = targetAi;
        aniParGroup->start();

        qDebug() << "before ani min max" << m_ta->getMin() << m_ta->getMax();
        qDebug() << "before ani zoom" << m_ta->getZoomOffsetMsecs();
        qDebug() << "before ani step drag" << m_ta->getStepInPx() << m_ta->getDragOffsetPx();
        qDebug() << "before ani day hour" << m_ta->getDayWidthInPx() << m_ta->getHourWidthInPx();
    }
        break;

    }


}


