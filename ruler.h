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

        m_ta->rulerWidth.store(rulerWidth); //std::rel_ack?

        //recalc curHourWidth
        m_ta->hourWidthInPx = m_ta->dayWidthInPx/24; //(rulerWidth/(-23 * m_ta->zoomRatio + 47));

        //drawing ruler
        p->setPen(m_plt.rulerBackground);
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(0, 0, rulerWidth,  hourLabelsBottomPos);

        //drawing lines and labels
        p->setPen(QPen(m_plt.labels, 2));
        p->setFont(font);

        int time{0};

        float i = m_ta->offsetInPx + m_ta->dragOffset + m_ta->dragOffsetCur;
        for (; time < 10; time++){
            if (i > m_ri.leftMargin && i < rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_ta->hourWidthInPx;
        }
        for (; time <= 24; time++){
            if (i > m_ri.leftMargin && i < rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_ta->hourWidthInPx;
        }
        // p->restore();

        m_ri.lastRender = std::chrono::system_clock::now();

    }

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();
        static const float wheelScrollRatio = 0.0005;
      //  static const int targetaoef = 1;


        float targetZoomRatio{m_ta->zoomRatio};
        targetZoomRatio += delta * wheelScrollRatio; //* curTargetaoef;
        if (targetZoomRatio < 1) targetZoomRatio = 1;
        else if (targetZoomRatio > 2) targetZoomRatio = 2;
//

        //zoomToCenter(targetZoomRatio);
        if (delta > 0)
            zoomInToCenter(targetZoomRatio);
            //zoomInToMousPos(targetZoomRatio, event->pos().rx());
        else
            zoomInToCenter(targetZoomRatio);
           // zoomOutToMousPos(targetZoomRatio, event->pos().rx());

        this->update();
        QGraphicsItem::wheelEvent(event);
    }


    void zoomInToCenter(float targetZoomRatio){
        //calculating target virtual width
        float targetDayWidthPx = m_ta->rulerWidth *  (1 + 24 * (targetZoomRatio - 1));
        //calculating center
        float realCenterPx = (m_ta->rulerWidth / 2) ;//- m_ta->dragOffset; //- m_ta->offsetInPx;
       // qDebug() << "target" << targetZoomRatio;
        int shift = m_ta->dragOffset;
     //   int zoomedShift = shift *  (1 + 24 * (targetZoomRatio - 1));
        float curRulerCenterPos = realCenterPx - m_ta->offsetInPx - m_ta->dragOffset;
        double posRatio = curRulerCenterPos  / m_ta->dayWidthInPx;
        qDebug() << "pos Ratio: " << posRatio << realCenterPx << m_ta->rulerWidth;
        float targetCenterPx = targetDayWidthPx * (posRatio);//+ m_ta->offsetInPx; // m_ta->offsetInPx*  (1 + 24 * (targetZoomRatio - 1));;

        int targetOffsetInPx = realCenterPx - (targetCenterPx);// - targetDayWidthPx/2) ;
         qDebug() << "pos rc tc dw" << posRatio << realCenterPx << targetCenterPx << targetDayWidthPx;
       // qDebug() << " rw" << realCenterPx << fullWidthCenterPx << shift << zoomedShift << targetOffsetInPx;
        m_ta->dayWidthInPx = targetDayWidthPx;
        m_ta->offsetInPx = targetOffsetInPx;
        m_ta->zoomRatio = targetZoomRatio;
      //  m_ta->dragOffset = zoomedShift;

        m_ta->hourWidthInPx = targetDayWidthPx/24;
    }

    void zoomOutToCenter(float targetZoomRatio){
        //calculating target virtual width
        float targetDayWidthPx = m_ta->rulerWidth *  (1 + 24 * (targetZoomRatio - 1));
        //calculating center
        int shift = m_ta->dragOffset;
        float realCenterPx = (m_ta->rulerWidth / 2);
       // qDebug() << "target" << targetZoomRatio;

        int zoomedShift = shift *  (1 + 24 * (targetZoomRatio - 1));

        float fullWidthCenterPx = (targetDayWidthPx / 2.) - zoomedShift;

        int targetOffsetInPx = realCenterPx - fullWidthCenterPx;
        qDebug() << " rw" << realCenterPx << fullWidthCenterPx << shift << zoomedShift << targetOffsetInPx;
        m_ta->dayWidthInPx = targetDayWidthPx;
        m_ta->offsetInPx = targetOffsetInPx;
        m_ta->zoomRatio = targetZoomRatio;

        m_ta->hourWidthInPx = fullWidthCenterPx/24;
    }

    void zoomInToMousPos(float targetZoomRatio, int mouseXPos){
        //calculating target virtual width
        float targetDayWidthPx = m_ta->rulerWidth *  (1 + 24 * (targetZoomRatio - 1));
        //calculating center
        int dragShift = m_ta->dragOffset;
        int mousePosShift = m_ta->rulerWidth/2 - mouseXPos;
        float realCenterPx = (m_ta->rulerWidth / 2) - dragShift - mousePosShift;
       // qDebug() << "target" << targetZoomRatio;

        int zoomedDragShift = dragShift *  (1 + 24 * (targetZoomRatio - 1));
        int zoomedMousePosShift = mousePosShift  *  (1 + 24 * (targetZoomRatio - 1));
        float fullWidthCenterPx = (targetDayWidthPx / 2.) - zoomedDragShift - zoomedMousePosShift;

        int targetOffsetInPx = realCenterPx - fullWidthCenterPx;
//        qDebug() << " rw" << realCenterPx << fullWidthCenterPx << shift << zoomedShift;
        m_ta->dayWidthInPx = targetDayWidthPx;
        m_ta->offsetInPx = targetOffsetInPx;
        m_ta->zoomRatio = targetZoomRatio;

        m_ta->hourWidthInPx = fullWidthCenterPx/24;
    }
    void zoomOutToMousPos(float targetZoomRatio, int mouseXPos){
        //calculating target virtual width
        float targetDayWidthPx = m_ta->rulerWidth *  (1 + 24 * (targetZoomRatio - 1));
        //calculating center
        int dragShift = m_ta->dragOffset;
        int mousePosShift = m_ta->rulerWidth/2 - mouseXPos;
        float realCenterPx = (m_ta->rulerWidth / 2) - dragShift - mousePosShift;
       // qDebug() << "target" << targetZoomRatio;

        int zoomedDragShift = dragShift *  (1 + 24 * (targetZoomRatio - 1));
        int zoomedMousePosShift = mousePosShift  *  (1 + 24 * (targetZoomRatio - 1));
        float fullWidthCenterPx = (targetDayWidthPx / 2.) - zoomedDragShift - zoomedMousePosShift;

        int targetOffsetInPx = realCenterPx - fullWidthCenterPx;
//        qDebug() << " rw" << realCenterPx << fullWidthCenterPx << shift << zoomedShift;
        m_ta->dayWidthInPx = targetDayWidthPx;
        m_ta->offsetInPx = targetOffsetInPx;
        m_ta->zoomRatio = targetZoomRatio;

        m_ta->hourWidthInPx = fullWidthCenterPx/24;
    }


    float zoomCurve(const float& targetZoomRaio){
        float&& res = (-23 * targetZoomRaio + 47);
        return res;
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override{
        mouseXPosOnPress = event->pos().rx();
        qDebug() << "mouse pressed";
        if (event->button() == Qt::MidButton){

            m_ta->dragOffset = 0;

        }
        else event->accept();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        m_ta->dragOffsetCur = curMouseXPos - mouseXPosOnPress;
       // qDebug() << "dragging: " << curMouseXPos << mouseXPosOnPress << m_ta->dragOffset;
        // m_ri._dragOffset = mouseXPosOnPress - curMouseXPos;
        //    qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        //m_ta->dragOffset = mouseXPosOnPress - curMouseXPos;
        //m_ri.dragOffset = mouseXPosOnPress - curMouseXPos;
        //    qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
            event->accept();
            m_ta->dragOffset += m_ta->dragOffsetCur;
            m_ta->dragOffsetCur = 0;
            qDebug() << "drag" << m_ta->dragOffset;
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
        int topLeftX = m_ta->getPxPosFromMsec(b.start);

        QRectF bkRect(topLeftX, m_ri.bookmarksTopY,
                      m_ta->getPxPosFromMsec(b.end) - topLeftX, m_ri.bookmarksBottomY - m_ri.bookmarksTopY);
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
