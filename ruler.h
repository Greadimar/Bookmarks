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
#include "timeconvertor.h"

class Ruler: public QGraphicsItem{
public:
    Ruler(const QSharedPointer<TimeConvertor>& c): timeConvertor(c){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);

        hour1xLabelsOffsetToCenter = fm.horizontalAdvance("1h")/2;
        hour2xLabelsOffsetToCenter = fm.horizontalAdvance("24h")/2;
        hourLabelsBottomPos = linesHeight + fm.height() + hourLabelsTopMargin;
    }
private:
    QSharedPointer<TimeConvertor> timeConvertor;
    QFont font{"Times", 10};
    double linesHeight = 30;
    const float hourLabelsTopMargin = 5;
    float hourLabelsBottomPos;
        float hour1xLabelsOffsetToCenter = 0;
    float hour2xLabelsOffsetToCenter = 0;
    float rulerMargin = 10;
    QRectF boundingRect() const override {
      //  int w = scene()->views().first()->width();
     //   return QRectF{QPointF{0.,0.},QPointF{static_cast<double>(w),30}};

    }
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
        QPointF curPos = scenePos();
        int viewWidth = scene()->views().first()->width();
        int rulerWidth = viewWidth - rulerMargin*2;
        timeConvertor->rulerWidth.store(rulerWidth); //std::rel_ack?

        float hourWidthInPx = rulerWidth/24.;
        p->setPen(QColor{"#f0fff0"});
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(rulerMargin, 0, rulerWidth,  hourLabelsBottomPos);
        p->setPen(QPen(QColor{Qt::black}, 2));

        p->setFont(font);
        p->save();
      //  p->scale(0.1, 0.1); //?

        int time{0};
        float i = rulerMargin;
        for (; time < 10; time++){
            p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
            p->drawText(QPointF{i - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            i += hourWidthInPx;
        }
        for (; time <= 24; time++){
            p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
            p->drawText(QPointF{i - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            i += hourWidthInPx;
        }



    }

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event){
//        double angle = wheel_event->angleDelta().y();
//                double factor = qPow(_zoom_factor_base, angle);
//                gentle_zoom(factor);
//                return true;
    }
};

class Scene: public QGraphicsScene
{
signals:

public slots:

public:
    Scene(){};
    int offset{0};
    int zoom = 1;
//    void drawBackground(QPainter *painter, const QRectF &rect){
//           // painter->setPen(QColor(Qt::green));
//          //  painter->drawRect(rect);
//        }
//    virtual void resizeEvent(QResizeEvent* event)
//     {

//         int const maximumMM = event->size().height() * toMM();
//         QFontMetrics fm(font());
//         int w = fm.width(QString::number(maximumMM)) + 20;
//         if (w != event->size().width())
//         {
//             QSize const newSize(w, event->size().height());
//             sizeChanged(newSize);
//             return setFixedSize(newSize);
//         }
//         return QGraphicsScene::resizeEvent(event);
//     }
//    void setOffset(int value)
//    {
//        offset = value;
//        update();
//    }
//    static qreal toMM()
//    {
//        return 25.4 / qApp->desktop()->logicalDpiY();
//    }

};

#endif // SCENE_H
