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




class RenderInfo: public QObject{
    Q_OBJECT
signals:
    void zoomRatioChanged();
    void dragOffsetahanged();
public:
    //Q_PROPERTY(float zoomRatio READ zoomR);
    Q_PROPERTY(float zoomRatio READ zoomRatio WRITE setZoomRatio NOTIFY zoomRatioChanged)
    Q_PROPERTY(int dragOffset READ dragOffset WRITE setDragOffset NOTIFY dragOffsetahanged);
    int rulerTopY{0};
    int rulerBottomY{100};
    const int bookmarksTopMargin = 10;
    const int leftMargin = 5;
    const int rightMargin = 5;
    int bookmarksTopY{10};
    int bookmarksBottomY{100};
    int bookmarksHeight = 90;
    int rulerWidth = 100;
    float _zoomRatio = 1; //1 - 24h, 2 = 1h
    int _dragOffset = 0;
    float zoomRatio() const;
    void setZoomRatio(float zoomRatio);
    int dragOffset() const;
    void setDragOffset(int dragOffset);

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
    }
private:
    const Palette& m_plt;
    RenderInfo& m_ri;


    QSharedPointer<TimeAxis> m_ta;
    QFont font{"Times", 10};

    QPointer<QPropertyAnimation> curZoomAni;
    QPointer<QPropertyAnimation> curOffsetAni;

    double linesHeight = 30;
    const float hourLabelsTopMargin = 5;
    float hourLabelsBottomPos;
    float hour1xLabelsOffsetToCenter = 0;
    float hour2xLabelsOffsetToCenter = 0;

    int mouseXPosOnPress{0};
    int curMouseXPos{0};

    QRectF boundingRect() const override {
        return QRectF{QPointF{0,0}, QPointF{static_cast<float>(m_ri.rulerWidth), static_cast<float>(m_ri.rulerBottomY)}};
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
        //  p->save();
        QPointF curPos = scenePos();
        int viewWidth = scene()->views().first()->width();
        m_ri.rulerWidth = viewWidth - m_ri.leftMargin - m_ri.rightMargin;
        m_ta->rulerWidth.store(m_ri.rulerWidth); //std::rel_ack?
       int updHourWidthInPx =  (m_ri.rulerWidth/(-23 * m_ri._zoomRatio + 47)); //hoursSeen = 47 - 23zoom        zoom 2 = 1h, zoom 1 = 24h

//        m_ta->updateHourWidthInPx();

        //calc 24h full width center
        float daycenterpos =  (m_ri.rulerWidth * updHourWidthInPx / m_ta->hourWidthInPx) / 2;
        m_ta->hourWidthInPx = updHourWidthInPx;
        //calc visual center
        float visuallcenterpos =
                (m_ri.rulerWidth + m_ri.leftMargin) / 2.;
        int offset = visuallcenterpos - daycenterpos;



        //drawing ruler
        p->setPen(m_plt.rulerBackground);
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(m_ri.leftMargin, 0, m_ri.rulerWidth,  hourLabelsBottomPos);

        //drawing lines and labels
        p->setPen(QPen(m_plt.labels, 2));
        p->setFont(font);

        msecs time{0};

        msecs curMin{m_ta->min.load(std::memory_order_relaxed)};
        msecs curMax{m_ta->max.load(std::memory_order_relaxed)};

        float pxStart = curPos.rx() + m_ri.leftMargin + offset;
        //float i = m_ri.leftMargin + m_ri.dragOffset();

//                for (; time < std::chrono::hours(10); time += m_ta->step){
//                    if (time > curMin && time < curMax){
//                        float px = pxStart + m_ta->getPxPosFromMsec(time);
//                        p->drawLine({px, curPos.ry()}, QPointF{px, curPos.ry() + linesHeight});
//                        p->drawText(QPointF{px - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(
//                                        std::chrono::duration_cast<std::chrono::hours>(time).count()));
//                    }
//                }
//                for (; time <= std::chrono::hours(24); time += m_ta->step){
//                    if (time > curMin && time < curMax){
//                        float px = pxStart + m_ta->getPxPosFromMsec(time);
//                        p->drawLine({px, curPos.ry()}, QPointF{px, curPos.ry() + linesHeight});
//                        p->drawText(QPointF{px - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(
//                                        std::chrono::duration_cast<std::chrono::hours>(time).count()));
//                    }
//                }

        for (; time < std::chrono::hours(10); time += m_ta->step){
            if (time > curMin && time < curMax){
                float px = pxStart + m_ta->getPxPosFromMsec(time);
                p->drawLine({px, curPos.ry()}, QPointF{px, curPos.ry() + linesHeight});
                p->drawText(QPointF{px - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(
                                std::chrono::duration_cast<std::chrono::hours>(time).count()));
            }
        }
        for (; time <= std::chrono::hours(24); time += m_ta->step){
            if (time > curMin && time < curMax){
                float px = pxStart + m_ta->getPxPosFromMsec(time);
                p->drawLine({px, curPos.ry()}, QPointF{px, curPos.ry() + linesHeight});
                p->drawText(QPointF{px - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(
                                std::chrono::duration_cast<std::chrono::hours>(time).count()));
            }
        }
        // p->restore();

        m_ri.lastRender = std::chrono::system_clock::now();
    }

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();
        static const float wheelScrollRatio = 0.0005;
        static const int targetaoef = 1;
        static int curTargetaoef = targetaoef;

        if (curZoomAni){
            if (curZoomAni->state() == QPropertyAnimation::State::Running){
                curTargetaoef /= 1.5;
            }
            else{
                curTargetaoef = targetaoef;
            }
            curZoomAni->stop();
            delete curZoomAni;
        }
        if (curOffsetAni){
            if (curOffsetAni->state() == QPropertyAnimation::State::Running){
                curTargetaoef /= 1.5;
            }
            else{
                curTargetaoef = targetaoef;
            }
            curOffsetAni->stop();
            delete curOffsetAni;
        }



        float targetZoomRatio{m_ri._zoomRatio};
        targetZoomRatio += delta * wheelScrollRatio; //* curTargetaoef;
        if (targetZoomRatio < 1) targetZoomRatio = 1;
        else if (targetZoomRatio > 2) targetZoomRatio = 2;

        float posRatio = curMouseXPos/static_cast<float>(m_ri.rulerWidth);

        // int targetOffset = (curMouseXPos - m_ri.rulerWidth) * (targetZoomRatio - 1); //* curTargetaoef;


        int wx = m_ri.rulerWidth / 2;
        float some{};

        int rulerWMax = m_ri.rulerWidth * (targetZoomRatio - 1);
        int offsetMax = -2 * rulerWMax + rulerWMax/24.;
        int targetOffset = m_ri.dragOffset();// posRatio * offsetMax;
        //if (posRatio > 0.5)
        targetOffset = (posRatio) * offsetMax;
        qDebug() << "target offset" << posRatio << targetOffset;


        auto max{m_ta->getMax()};
        auto min{m_ta->getMin()};
        int wX = m_ta->getDuration().count();
        int curX = m_ta->getMsecFromPxPosition(curMouseXPos).count();

        auto precMinX = (curX - min) / static_cast<float>(wX);
        auto precMaxX = (max - curX) / static_cast<float>(wX);

        auto targetMin = (curX - wX * 1.05*precMinX);
        auto targetMax = (curX + wx * 1.05*precMaxX);
        qDebug() << "cur m m" << min << max;
        qDebug() << "dur" << wX;
        qDebug() << "cur mouse pos in msecs" <<curMouseXPos << curX;
        qDebug() << "target m m" << targetMin << targetMax;

       m_ta->setMax(targetMax);
        m_ta->setMin(targetMin);

     //   QPropertyAnimation *minAni = new QPropertyAnimation(m_ta.data(), "min");

//        QPropertyAnimation *zoomAni = new QPropertyAnimation(&m_ri, "zoomRatio");
//        minAni->setDuration(300);
//        minAni->setStartValue(m_ta->getMin());
//        minAni->setEndValue(targetMin);
//        minAni->start();

//        QPropertyAnimation *maxAni = new QPropertyAnimation(m_ta.data(), "max");

        QPropertyAnimation *zoomAni = new QPropertyAnimation(&m_ri, "zoomRatio");
        zoomAni->setDuration(300);
        zoomAni->setStartValue(m_ri.zoomRatio());
        zoomAni->setEndValue(targetZoomRatio);
        zoomAni->start();

//        QPropertyAnimation *offsetAni = new QPropertyAnimation(&m_ri, "dragOffset");
//        offsetAni->setDuration(300);
//        offsetAni->setStartValue(m_ri._dragOffset);
//        offsetAni->setEndValue(targetOffset);
//        offsetAni->start();
        // int targetOffset = {m_ri}

        //m_ri.zoomRatio = targetZoomRatio;
        this->update();
        QGraphicsItem::wheelEvent(event);
    }

    //    void mousePressEvent(QGraphicsSceneMouseEvent* event) override{
    //        mouseXPosOnPress = event->pos().rx();
    //        qDebug() << "mouse pressed";
    //        if (event->button() == Qt::MidButton) m_ri.dragOffset = 0;
    //        else event->accept();
    //    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        m_ri._dragOffset = mouseXPosOnPress - curMouseXPos;
        //    qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        //m_ri.dragOffset = mouseXPosOnPress - curMouseXPos;
        //    qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
    //    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
    //        event->accept();
    //    }
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
        QRectF bkRect(m_ta->getPxPosFromMsec(b.start), m_ri.bookmarksTopY,
                      m_ta->getPxPosFromMsec(b.end), m_ri.bookmarksBottomY);
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
        QPoint posTopLeft{m_ta->getPxPosFromMsec(mb.start), m_ri.bookmarksTopY};

        int w = posTopLeft.rx() + multiBoookmarkWidth - borderWidth;
        int h = m_ri.bookmarksHeight;
        qDebug() << "painting bk: " << posTopLeft << w << h;
        QRectF bkRect(posTopLeft.rx(), posTopLeft.ry(), w, h);
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
