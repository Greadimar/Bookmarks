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

class Ruler: public QGraphicsObject{
public:
    Ruler(const Palette& plt, RenderInfo& ri, const QPointer<TimeAxis>& c);
private:
    const Palette& m_plt;
    RenderInfo& m_ri;
    enum class Animation{
        noAnimation = 0,
        simpleAnimation = 1,
        animationWithInertion = 2
    };
    Animation animation{Animation::simpleAnimation};


    QPointer<TimeAxis> m_ta;
    QFont font{"Times", 10};


    int animationDuration{300};
    QPropertyAnimation* aniZoomOffset;
    QPropertyAnimation* aniCurOffset;
    QPropertyAnimation* aniMin;
    QPropertyAnimation* aniMax;
    QPropertyAnimation* aniAxisInfo;
    QParallelAnimationGroup* aniParGroup;

    //        QAnimationGroup* g = new QAnimationGroup();

    double linesHeight = 30;
    const float hourLabelsTopMargin = 5;
    float hourLabelsBottomPos;
    float hour1xLabelsOffsetToCenter = 0;
    float hour2xLabelsOffsetToCenter = 0;

    int mouseXPosOnPress{0};
    int curMouseXPos{0};
    void zoomToCenter(float targetZoomRatio);

    QRectF boundingRect() const override {
        return QRectF{QPointF{0,0}, QPointF{static_cast<float>(m_ta->rulerWidth), static_cast<float>(m_ri.rulerBottomY)}};
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override;

    QPropertyAnimation* makeAnimation(QObject* obj, const QString& propValue);


private: //events
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();

        static const float targetZoomInRatio{1.05};
        static const float targetZoomOutRatio{0.95};

        if (delta > 0)
            zoomToCenter(targetZoomInRatio);
        else
            zoomToCenter(targetZoomOutRatio);

        this->update();
        QGraphicsItem::wheelEvent(event);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override{
        mouseXPosOnPress = event->pos().rx();
        if (event->button() == Qt::MidButton){

            auto hourw = static_cast<float>(m_ta->rulerWidth/24);
            auto dayw = static_cast<float>(m_ta->rulerWidth);

            AxisInfo ai{hourw, dayw, hourw, 0, 0};
            m_ta->setAi(ai);

            m_ta->step = TimeInfo::msecsInhour.count();
            m_ta->setMin(0);
            m_ta->setMax(m_ta->msecFromPx(m_ta->getAi().dayWidthInPx));

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
        curMouseXPos = event->pos().rx();
        AxisInfo ai = m_ta->getAi();
        ai.dragOffsetCur = mouseXPosOnPress - curMouseXPos;
        m_ta->setAi(ai);
        m_ta->setMin(m_ta->getZoomOffsetMsecs() + m_ta->msecFromPx(ai.dragOffset + ai.dragOffsetCur));
        m_ta->setMax(m_ta->getMin() + m_ta->msecFromPx(m_ta->rulerWidth));

    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{
       // qDebug() << "hv";
        curMouseXPos = mapToScene(event->pos()).rx();
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
        event->accept();
        AxisInfo ai = m_ta->getAi();
        ai.dragOffset += ai.dragOffsetCur;
        ai.dragOffsetCur = 0;
        m_ta->setAi(ai);

    }
    AxisInfo createDefaultAi(){

    }
};


#endif // SCENE_H
