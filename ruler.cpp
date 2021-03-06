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
        auto hourw = rulerWidth/24.;
        auto dayw = static_cast<float>(rulerWidth);


        m_ta->setHourWidthInPx(hourw);
        m_ta->setDayWidthInPx(dayw);
        m_ta->setStepInPx(hourw);
        m_ta->setDragOffsetPx(0);
        m_ta->setDragOffsetCurPx(0);
    });

    //init animation
    aniZoomOffset = makeZoomAnimation(m_ta, "zoomOffMsecs");
    aniMin = makeZoomAnimation(m_ta, "min");
    aniMax = makeZoomAnimation(m_ta, "max");
    aniDragOffset = makeZoomAnimation(m_ta, "dragOffset");
    aniDragCurOffset = makeZoomAnimation(m_ta, "dragOffsetCur");

    aniInertDragOffset = makeInertAnimation(m_ta, "dragOffset");
    aniInertMin = makeInertAnimation(m_ta, "min");
    aniInertMax = makeInertAnimation(m_ta, "max");


    aniZoomParGroup = new QParallelAnimationGroup(this);
    aniZoomParGroup->addAnimation(aniZoomOffset);
    aniZoomParGroup->addAnimation(aniMin);
    aniZoomParGroup->addAnimation(aniMax);
    aniZoomParGroup->addAnimation(aniDragOffset);

    aniInertParGroup = new QParallelAnimationGroup(this);
    aniInertParGroup->addAnimation(aniInertMin);
    aniInertParGroup->addAnimation(aniInertMax);
    aniInertParGroup->addAnimation(aniInertDragOffset);


    aniZoomOffset->setDuration(animationDuration);
    aniMin->setDuration(animationDuration);
    aniMax->setDuration(animationDuration);  
    aniZoomOffset->setDuration(animationDuration);
    aniDragOffset->setDuration(animationDuration);
    aniDragCurOffset->setDuration(animationDuration);

    connect(aniZoomParGroup, &QParallelAnimationGroup::finished, this, [=](){
        dbg("after");
    });


    //init timer for calculating acceleration on drag


}

void Ruler::setAnimation(const Animation &value)
{
    animation = value;
}

void Ruler::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
    // p->save();
    QPointF curPos = scenePos();
    int viewWidth = scene()->views().first()->width();
    int rulerWidth = viewWidth;
    float resizeSinceLastRender = rulerWidth / static_cast<float>(m_ta->rulerWidth);

    m_ta->updScaleByMinMax();

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
            float curTimePx = m_ta->pxPosFromMsec(time);
            p->drawLine( QPointF{rx + curTimePx, curPos.ry()}, QPointF{rx + curTimePx, curPos.ry() + linesHeight});
            p->drawText(QPointF{curTimePx - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(h));
        }
        h++;

    }
    for (; time <= msecIn24h; time += m_ta->step){
        if (time >= min && time <= max){
            int curTimePx = m_ta->pxPosFromMsec(time);
            p->drawLine( QPointF{rx + curTimePx, curPos.ry()}, QPointF{rx + curTimePx, curPos.ry() + linesHeight});
            p->drawText(QPointF{curTimePx - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(h));
        }
        h++;
    }
    // p->restore();

    m_ri.lastRender = std::chrono::system_clock::now();

}

QPropertyAnimation *Ruler::makeZoomAnimation(QObject *obj, const QString &propValue){
    QPropertyAnimation* anim = new QPropertyAnimation(obj, propValue.toUtf8(), this);
    anim->setEasingCurve(zoomCurve);
    return anim;
}
QPropertyAnimation *Ruler::makeInertAnimation(QObject *obj, const QString &propValue){
    QPropertyAnimation* anim = new QPropertyAnimation(obj, propValue.toUtf8(), this);
    anim->setEasingCurve(inertCurve);
    return anim;
}

void Ruler::zoomToCenter(float targetZoomRatio){

    int curPosInMsecs = m_ta->getMin() + m_ta->msecFromPx(curMouseXPos);
    auto targetDayWidth = m_ta->getDayWidthInPx() * targetZoomRatio;
    auto targetHourWidth = targetDayWidth/24;
    auto targetStepInPx = m_ta->getStepInPx() * targetZoomRatio;
    float targetMousePosMsec = m_ta->msecFromPx(curMouseXPos, targetHourWidth);
    int targetZoomOffset = curPosInMsecs - targetMousePosMsec;
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
      //  dbg("before");
        aniMax->setStartValue(m_ta->getMax());
        aniMax->setEndValue(targetMax);
        aniMin->setStartValue(m_ta->getMin());
        aniMin->setEndValue(targetMin);
        aniZoomOffset->setStartValue(m_ta->getZoomOffsetMsecs());
        aniZoomOffset->setEndValue(targetZoomOffset);
        aniDragOffset->setStartValue(m_ta->getDragOffsetPx());
        aniDragOffset->setEndValue(targetDragOffset);
        if (aniZoomParGroup->state() == QParallelAnimationGroup::State::Running) aniZoomParGroup->stop();
        aniZoomParGroup->start();

    }
        break;

    }


}


