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


class Ruler: public QGraphicsObject{
public:
    Ruler(const Palette& plt, RenderInfo& ri, const QSharedPointer<TimeAxis>& c);
private:
    const Palette& m_plt;
    RenderInfo& m_ri;


    QSharedPointer<TimeAxis> m_ta;
    QFont font{"Times", 10};

    QPointer<QPropertyAnimation> curZoomAni;
    QPointer<QPropertyAnimation> curOffsetAni;

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

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();

        float targetZoomInRatio{1.05};
        float targetZoomOutRatio{0.95};

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

            m_ta->dragOffset = 0;
            m_ta->zoomOffsetMsecs = 0;
            m_ta->dayWidthInPx = m_ta->rulerWidth;
            m_ta->hourWidthInPx = m_ta->dayWidthInPx / 24;
            m_ta->step = TimeInfo::msecsInhour.count();
            m_ta->stepInPx = m_ta->step * (m_ta->hourWidthInPx /static_cast<float>(TimeInfo::msecsInhour.count()));

            m_ta->setMin(0);
            m_ta->setMax(m_ta->msecFromPx(m_ta->dayWidthInPx));

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
        m_ta->dragOffsetCur = mouseXPosOnPress - curMouseXPos;

        m_ta->setMin(m_ta->zoomOffsetMsecs + m_ta->msecFromPx(m_ta->dragOffset + m_ta->dragOffsetCur));
        m_ta->setMax(m_ta->getMin() + m_ta->msecFromPx(m_ta->rulerWidth));

    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{
       // qDebug() << "hv";
        curMouseXPos = mapToScene(event->pos()).rx();
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
        event->accept();
        m_ta->dragOffset += m_ta->dragOffsetCur;
        m_ta->dragOffsetCur = 0;

    }
};


#endif // SCENE_H
