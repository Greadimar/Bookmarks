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
struct Palette{
    QColor rulerBorders{"#ed9121"};
    QColor rulerLines{Qt::black};
    QColor rulerBackground{"#FEFED3"};
    QColor labels{Qt::black};
    QColor singleBookmarksBorders{"#024064"};
    QColor singleBookmarksBackground{"#3CB3C0"};
    QColor multiBookmarksBorders{"#37782C"};
    QColor multiBookmarksBackground{"#64BB6A"};
};




struct RenderInfo{

    int rulerTopY{0};
    int rulerBottomY{100};
    const int bookmarksTopMargin = 10;
    const int leftMargin = 5;
    const int rightMargin = 5;
    int bookmarksTopY{10};
    int bookmarksBottomY{100};
    int bookmarksHeight = 90;



    //render info
    const msecs renderStep{33}; //~30fps
    std::chrono::system_clock::time_point lastRender;
};

class Ruler: public QGraphicsObject{
public:
    Ruler(const Palette& plt, RenderInfo& ri, const QSharedPointer<TimeAxis>& c): m_plt(plt), m_ri(ri), m_ta(c){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);

        hour1xLabelsOffsetToCenter = fm.horizontalAdvance("1h")/2;
        hour2xLabelsOffsetToCenter = fm.horizontalAdvance("24h")/2;
        hourLabelsBottomPos = linesHeight + fm.height() + hourLabelsTopMargin;
        ri.rulerBottomY = hourLabelsBottomPos;
        ri.bookmarksTopY = ri.rulerBottomY + ri.bookmarksTopMargin;

        setAcceptHoverEvents(true);

        //set initial params
        QTimer::singleShot(10, this, [=](){
            int viewWidth = scene()->views().first()->width();
            int rulerWidth = viewWidth;// - m_ri.leftMargin - m_ri.rightMargin;
            m_ta->rulerWidth.store(rulerWidth);
            m_ta->dayWidthInPx = rulerWidth;
            m_ta->hourWidthInPx = m_ta->dayWidthInPx / 24;
            m_ta->stepInPx = m_ta->hourWidthInPx;
        });
    }
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

    QRectF boundingRect() const override {
        return QRectF{QPointF{0,0}, QPointF{static_cast<float>(m_ta->rulerWidth), static_cast<float>(m_ri.rulerBottomY)}};
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
        //  p->save();
        //  p->save();
        QPointF curPos = scenePos();
        int viewWidth = scene()->views().first()->width();
        int rulerWidth = viewWidth;//- m_ri.leftMargin - m_ri.rightMargin;
        float resizeSinceLastRender = rulerWidth / static_cast<float>(m_ta->rulerWidth);
        m_ta->stepInPx = m_ta->stepInPx * resizeSinceLastRender;
        m_ta->hourWidthInPx = m_ta->hourWidthInPx * resizeSinceLastRender;
        m_ta->dayWidthInPx = m_ta->hourWidthInPx * 24;
        m_ta->rulerWidth.store(rulerWidth); //std::rel_ack?

        //recalc curHourWidth
        //  m_ta->hourWidthInPx =(rulerWidth/(-23 * m_ta->zoomRatio + 47));

        //drawing ruler
        p->setPen(m_plt.rulerBackground);
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(0, 0, rulerWidth,  hourLabelsBottomPos);

        //drawing lines and labels
        p->setPen(QPen(m_plt.labels, 2));
        p->setFont(font);

        int time{0};
        int min{m_ta->getMin()};
        int max{m_ta->getMax()};



        // int msecInPx{static_cast<int>(m_ta->getVisibleDuration().count()/ rulerWidth)};


        //        float i = m_ta->offsetInPx + m_ta->dragOffset + m_ta->dragOffsetCur;
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

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();
        static const float wheelScrollRatio = 0.0005;
        //  static const int targetaoef = 1;


        float targetZoomInRatio{1.05};
        float targetZoomOutRatio{0.95};

        if (delta > 0)
            zoomToCenter(targetZoomInRatio);
        else
            zoomToCenter(targetZoomOutRatio);

        this->update();
        QGraphicsItem::wheelEvent(event);
    }
    void zoomToCenter(float targetZoomRatio){
        //   m_ta->zoom//Ratio = targetZoomRatio;
        int curPosInMsecs = m_ta->getMin() + m_ta->msecFromPx(curMouseXPos);
        m_ta->dayWidthInPx = m_ta->dayWidthInPx * targetZoomRatio;
        m_ta->hourWidthInPx = m_ta->dayWidthInPx/24;
        m_ta->stepInPx = m_ta->stepInPx * targetZoomRatio;
        float targetMousePosMsec = m_ta->msecFromPx(curMouseXPos);


        m_ta->zoomOffsetMsecs = curPosInMsecs - targetMousePosMsec;
        int targetMin = m_ta->zoomOffsetMsecs ;
        int targetMax = targetMin + m_ta->msecFromPx(m_ta->rulerWidth);
        m_ta->setMin(targetMin);
        m_ta->setMax(targetMax);
        m_ta->dragOffset = 0;
    }


    float zoomCurve(const float& targetZoomRaio){
        float&& res = (-23 * targetZoomRaio + 47);
        return res;
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override{
        mouseXPosOnPress = event->pos().rx();
        if (event->button() == Qt::MidButton){

            m_ta->dragOffset = 0;
            m_ta->dayWidthInPx = m_ta->rulerWidth;
            m_ta->hourWidthInPx = m_ta->dayWidthInPx / 24;
            m_ta->step = TimeInfo::msecsInhour.count();
            m_ta->stepInPx = m_ta->step * (m_ta->hourWidthInPx /static_cast<float>(TimeInfo::msecsInhour.count()));

            m_ta->setMin(0);
            m_ta->setMax(m_ta->msecFromPx(m_ta->dayWidthInPx));

        }
        else event->accept();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override{

        curMouseXPos = event->pos().rx();
        m_ta->dragOffsetCur = mouseXPosOnPress - curMouseXPos;

        m_ta->setMin(m_ta->zoomOffsetMsecs + m_ta->msecFromPx(m_ta->dragOffset + m_ta->dragOffsetCur));
        m_ta->setMax(m_ta->getMin() + m_ta->msecFromPx(m_ta->rulerWidth));

    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{
        curMouseXPos = mapToScene(event->pos()).rx();
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
        event->accept();
        m_ta->dragOffset += m_ta->dragOffsetCur;
        m_ta->dragOffsetCur = 0;

    }
};

class BookmarksLine: public QGraphicsItem{
public:
    BookmarksLine(const Palette& plt, QPointer<BookmarkManager> bmkMngr, RenderInfo& ri, const QSharedPointer<TimeAxis>& t):
        m_plt(plt), m_bmkMngr(bmkMngr), m_ri(ri), m_ta(t){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);
        ri.bookmarksBottomY = ri.bookmarksTopY + ri.bookmarksHeight;
    }
private:
    const Palette& m_plt;
    QPointer<BookmarkManager> m_bmkMngr;
    const RenderInfo& m_ri;
    QSharedPointer<TimeAxis> m_ta;
    QFont font{"Times", 10};
    const int multiBoookmarkWidth = 100;
    const int borderWidth = 2;
    QRectF boundingRect() const override {
        //  int w = scene()->views().first()->width();
        //   return QRectF{QPointF{0.,0.},QPointF{static_cast<double>(w),30}};

    }
    void drawBookmark(QPainter* p, const MultiBookmark& mbk){
        if (mbk.count == 1){
            p->setPen(QPen(m_plt.singleBookmarksBorders));

        }
    }
    void paint(QPainter *p, const QStyleOptionGraphicsItem *st, QWidget *) override {
        QPointF curPos = scenePos();
        int viewWidth = scene()->views().first()->width();
        int width = viewWidth - m_ri.leftMargin - m_ri.rightMargin;
        bool expectedIsSwapping = true;
        //        m_bmkMngr->toSwap.store(true);

        //        while (m_bmkMngr->toSwap.load(std::memory_order_seq_cst)){
        //           // QApplication::processEvents();
        //        }

        //auto& readyBufPtr = m_bmkMngr->readyBuffer;
        //        auto vecToDisplay = readyBufPtr.load();
        //  while(m_bmkMngr->stale.exchange(true)){
        //       QGuiApplication::processEvents();
        ///  }
        //

        //while (m_bmkMngr->stale.exchange(true));
        //if(m_bmkMngr->displayBufIsBusy.exchange(true)) return;
        // qDebug() << "paint: get bufPtr LOCK";
        // auto& displayBufPtr = m_bmkMngr->displayBuffer;
        m_bmkMngr->mutex.lock();
        auto&& vec = *m_bmkMngr->displayBuffer;

        for( auto& b: vec){
            if (b.count == 1) paintBk(b, p, st);
            else paintMbk(b, p, st);
        }
        m_bmkMngr->mutex.unlock();
        m_bmkMngr->displayBufIsBusy.store(false);
        //          qDebug() << "paint: UNLOCK";
    }

    void paintBk(const Bookmark& b, QPainter* p, const QStyleOptionGraphicsItem *st){
        p->setPen(QPen(m_plt.singleBookmarksBorders, 2));
        int topLeftX = m_ta->pxPosFromMsec(b.start);

        QRectF bkRect(topLeftX, m_ri.bookmarksTopY,
                      m_ta->pxPosFromMsec(b.end) - topLeftX, m_ri.bookmarksBottomY - m_ri.bookmarksTopY);
        QPainterPath pp;
        pp.addRoundedRect(bkRect, 5, 5);
        p->fillPath(pp, QBrush(m_plt.singleBookmarksBackground));
        p->drawRoundedRect(bkRect, 5, 5);
        QString name{b.name};

        QPointF namePos{bkRect.x() + bkRect.width()/2 + st->fontMetrics.horizontalAdvance(name)/2,
                    bkRect.y() + 2*bkRect.height()/3};
        p->drawText(namePos, name);
    }
    void paintMbk(const MultiBookmark& mb, QPainter* p, const QStyleOptionGraphicsItem *st){
        p->setPen(QPen(m_plt.multiBookmarksBorders, 2));
        QPoint posTopLeft{m_ta->pxPosFromMsec(mb.start), m_ri.bookmarksTopY};

        int w = multiBoookmarkWidth - borderWidth;
        int h = m_ri.bookmarksHeight;
        // qDebug() << "painting bk: " << posTopLeft << w << h;
        QRectF bkRect(posTopLeft.rx(), posTopLeft.ry(), w, h - posTopLeft.ry());
        QPainterPath pp;
        pp.addRoundedRect(bkRect, 5, 5);
        // p->fillPath(pp, QBrush(m_plt.multiBookmarksBackground));
        p->drawRect(bkRect);
        QString name = {QString::number(mb.count)};



        QPointF namePos{bkRect.x() + bkRect.width()/2 - st->fontMetrics.horizontalAdvance(name)/2,
                    bkRect.y() + 2*bkRect.height()/3};
        p->drawText(namePos, name);

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
