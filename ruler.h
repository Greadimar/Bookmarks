#ifndef SCENE_H
#define SCENE_H
#include <QGraphicsScene>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QDebug>
#include <QFontMetrics>
#include "timeaxis.h"
#include "bookmarkmanager.h"
#include <QPointer>
#include <QGraphicsSceneWheelEvent>
#include <QStyleOptionGraphicsItem>
#include <variant>
#include <QPropertyAnimation>
#include <QTimer>
#include "palette.h"
#include "renderinfo.h"
#include <QParallelAnimationGroup>
#include <QElapsedTimer>

class Ruler: public QGraphicsObject{
public:
    enum class Animation{
        noAnimation = 0,
        simpleAnimation = 1,
        animationWithInertion = 2
    };
    Ruler(const Palette& plt, RenderInfo& ri, const QPointer<TimeAxis>& c);
    void setAnimation(const Animation &value);

private:
    const Palette& m_plt;
    RenderInfo& m_ri;

    Animation animation{Animation::noAnimation};
    QEasingCurve zoomCurve{QEasingCurve::Linear};
    QEasingCurve inertCurve{QEasingCurve::OutCirc};


    QPointer<TimeAxis> m_ta;
    QFont font{"Times", 10};


    int animationDuration{400};
    QPropertyAnimation* aniZoomOffset;
    QPropertyAnimation* aniDragOffset;
    QPropertyAnimation* aniDragCurOffset;
    QPropertyAnimation* aniMin;
    QPropertyAnimation* aniMax;
    QParallelAnimationGroup* aniZoomParGroup;

    QPropertyAnimation* aniInertDragOffset;
    QPropertyAnimation* aniInertMin;
    QPropertyAnimation* aniInertMax;

    QParallelAnimationGroup* aniInertParGroup;

    //        QAnimationGroup* g = new QAnimationGroup();

    double linesHeight = 30;
    const float hourLabelsTopMargin = 5;
    float hourLabelsBottomPos;
    float hour1xLabelsOffsetToCenter = 0;
    float hour2xLabelsOffsetToCenter = 0;

    int mouseXPosOnPress{0};
    QElapsedTimer timerOnPress;
    int curMouseXPos{0};

    void zoomToCenter(float targetZoomRatio);

    QRectF boundingRect() const override {
        return QRectF{QPointF{0,0}, QPointF{static_cast<float>(m_ta->rulerWidth), static_cast<float>(m_ri.rulerBottomY)}};
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override;

    QPropertyAnimation* makeZoomAnimation(QObject* obj, const QString& propValue);
    QPropertyAnimation* makeInertAnimation(QObject* obj, const QString& propValue);


private: //events
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        static int delta{event->delta()};
        int curDelta = event->delta();
        curMouseXPos = event->pos().rx();

        //init zoomRatio constants
        static const float zoomInRatio{1.05};
        static const float zoomOutRatio{0.95};
        static const float zoomInStep{0.05};
        static const float zoomOutStep{0.01};
        static float targetZoomInRatio = zoomInRatio;
        static float targetZoomOutRatio = zoomOutRatio;

        //reset dragging
        auto targetDragOffsetPx(m_ta->getDragOffsetPx() + m_ta->getDragOffsetCurPx());
        auto targetDragOffsetCurPx(0);
        m_ta->setDragOffsetPx(targetDragOffsetPx);
        m_ta->setDragOffsetCurPx(targetDragOffsetCurPx);
        mouseXPosOnPress = curMouseXPos;

        //set targetZoom
        if (animation != Animation::noAnimation){
            if ((curDelta > 0 && delta < 0) || (curDelta < 0 && delta > 0)){    //stop animation for switching direction
                aniZoomParGroup->stop();
            }
            if (aniZoomParGroup->state() == QParallelAnimationGroup::State::Running){
                targetZoomInRatio += zoomInStep;
                targetZoomOutRatio -= zoomOutStep;
            }
            else{
                targetZoomInRatio = zoomInRatio;
                targetZoomOutRatio = zoomOutRatio;
            }
        }

        delta = curDelta;

        if (delta > 0)
            zoomToCenter(targetZoomInRatio);
        else
            zoomToCenter(targetZoomOutRatio);

        this->update();
        QGraphicsItem::wheelEvent(event);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override{
        mouseXPosOnPress = event->pos().rx();
        timerOnPress.restart();
        if (event->button() == Qt::MidButton){
            aniZoomParGroup->stop();
            aniInertDragOffset->stop();
            auto hourw = m_ta->rulerWidth/24.;
            auto dayw = static_cast<float>(m_ta->rulerWidth);
            m_ta->setHourWidthInPx(hourw);
            m_ta->setDayWidthInPx(dayw);
            m_ta->setDragOffsetPx(0);
            m_ta->setDragOffsetCurPx(0);

            m_ta->step = TimeInfo::msecsInhour.count();
            m_ta->setMin(0);
            m_ta->setMax(m_ta->msecFromPx(m_ta->getDayWidthInPx()));

        }

        if (event->button() == Qt::RightButton){
            int m = m_ta->msecFromPx(curMouseXPos) + m_ta->getMin();
            qDebug() << "msec pos : " <<curMouseXPos << m << toHours(m);
            qDebug() << "inv : " <<m_ta->pxPosFromMsec(m);
        }
        else event->accept();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override{
        // qDebug() << "mv";

        aniZoomParGroup->stop();
        curMouseXPos = event->pos().rx();
        m_ta->setDragOffsetCurPx(mouseXPosOnPress - curMouseXPos);
        m_ta->setMin(m_ta->getZoomOffsetMsecs() + m_ta->msecFromPx(m_ta->getDragOffsetPx() + m_ta->getDragOffsetCurPx()));
        m_ta->setMax(m_ta->getMin() + m_ta->msecFromPx(m_ta->rulerWidth));
        return;
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        // qDebug() << "hv" << curMouseXPos;
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
        if (event->button() == Qt::MidButton){
            return;
        }
        event->accept();
        aniZoomParGroup->stop();
        aniInertParGroup->stop();
        auto targetDragOffsetPx(m_ta->getDragOffsetPx() + m_ta->getDragOffsetCurPx());
        auto targetDragOffsetCurPx(0);
        m_ta->setDragOffsetPx(targetDragOffsetPx);
        m_ta->setDragOffsetCurPx(targetDragOffsetCurPx);


        switch (animation){
        case Animation::animationWithInertion: {

            int curReleaseMouseXPos = mapToScene(event->pos()).rx();
            float startVelocity =  (mouseXPosOnPress - curReleaseMouseXPos) / static_cast<double>(timerOnPress.elapsed());

            float targetInertionDragOffset = targetDragOffsetPx + startVelocity*animationDuration;

            qDebug() << "Inertia: " << curReleaseMouseXPos << startVelocity << targetInertionDragOffset;

            aniInertDragOffset->setStartValue(targetDragOffsetPx);
            aniInertDragOffset->setEndValue(targetInertionDragOffset);
            aniInertMin->setStartValue(m_ta->getMin());
            auto targetMin = m_ta->getZoomOffsetMsecs() + m_ta->msecFromPx(targetInertionDragOffset);
            aniInertMin->setEndValue(targetMin);
            aniInertMax->setStartValue(m_ta->getMax());
            aniInertMax->setEndValue(targetMin + m_ta->msecFromPx(m_ta->rulerWidth));
            aniInertParGroup->start();
        }
            break;
        default:

            break;
        }


    }

    void dbg(QString n){
        qDebug() << n << "ani min max" << m_ta->getMin() << m_ta->getMax();
        qDebug() << n <<" ani zoom" << m_ta->getZoomOffsetMsecs();
        qDebug() << n <<" ani step drag" << m_ta->getStepInPx() << m_ta->getDragOffsetPx();
        qDebug() << n <<" ani day hour" << m_ta->getDayWidthInPx() << m_ta->getHourWidthInPx();
    }
};


#endif // SCENE_H
