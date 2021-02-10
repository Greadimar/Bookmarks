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
    void dragOffsetChanged();
public:
    //Q_PROPERTY(float zoomRatio READ zoomR);
    Q_PROPERTY(float zoomRatio READ zoomRatio WRITE setZoomRatio NOTIFY zoomRatioChanged)
    Q_PROPERTY(int dragOffset READ dragOffset WRITE setDragOffset NOTIFY dragOffsetChanged);
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
    Ruler(const Palette& plt, RenderInfo& ri, const QSharedPointer<TimeConvertor>& c): m_plt(plt), m_ri(ri), m_tc(c){
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
    QSharedPointer<TimeConvertor> m_tc;
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
        m_tc->rulerWidth.store(m_ri.rulerWidth); //std::rel_ack?
        m_tc->hourWidthInPx = (m_ri.rulerWidth/(-23 * m_ri._zoomRatio + 47)); //y = 47 - 23x
        //drawing ruler
        p->setPen(m_plt.rulerBackground);
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(m_ri.leftMargin, 0, m_ri.rulerWidth,  hourLabelsBottomPos);

        //drawing lines and labels
        p->setPen(QPen(m_plt.labels, 2));
        p->setFont(font);

        int time{0};

        float i = m_ri.leftMargin + m_ri.dragOffset();
        for (; time < 10; time++){
            if (i > m_ri.leftMargin && i < m_ri.rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_tc->hourWidthInPx;
        }
        for (; time <= 24; time++){
            if (i > m_ri.leftMargin && i < m_ri.rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_tc->hourWidthInPx;
        }
        // p->restore();

        m_ri.lastRender = std::chrono::system_clock::now();
    }

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();
        static const float wheelScrollRatio = 0.0005;
        static const int targetCoef = 1;
        static int curTargetCoef = targetCoef;

        if (curZoomAni){
            if (curZoomAni->state() == QPropertyAnimation::State::Running){
                curTargetCoef /= 1.5;
            }
            else{
                curTargetCoef = targetCoef;
            }
            curZoomAni->stop();
            delete curZoomAni;
        }
        if (curOffsetAni){
            if (curOffsetAni->state() == QPropertyAnimation::State::Running){
                curTargetCoef /= 1.5;
            }
            else{
                curTargetCoef = targetCoef;
            }
            curOffsetAni->stop();
            delete curOffsetAni;
        }



        float targetZoomRatio{m_ri._zoomRatio};
        targetZoomRatio += delta * wheelScrollRatio; //* curTargetCoef;
        if (targetZoomRatio < 1) targetZoomRatio = 1;
        else if (targetZoomRatio > 2) targetZoomRatio = 2;

        float posRatio = curMouseXPos/static_cast<float>(m_ri.rulerWidth);

        int targetOffset = (posRatio * m_ri.rulerWidth - m_ri.rulerWidth) * (targetZoomRatio - 1); //* curTargetCoef;

        qDebug() << "target offset" << posRatio << targetOffset;






        QPropertyAnimation *zoomAni = new QPropertyAnimation(&m_ri, "zoomRatio");
        zoomAni->setDuration(300);
        zoomAni->setStartValue(m_ri._zoomRatio);
        zoomAni->setEndValue(targetZoomRatio);
        zoomAni->start();


        QPropertyAnimation *offsetAni = new QPropertyAnimation(&m_ri, "dragOffset");
        offsetAni->setDuration(300);
        offsetAni->setStartValue(m_ri._dragOffset);
        offsetAni->setEndValue(targetOffset);
        offsetAni->start();
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
        qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{

        curMouseXPos = mapToScene(event->pos()).rx();
        //m_ri.dragOffset = mouseXPosOnPress - curMouseXPos;
        qDebug() << "mousePos" << curMouseXPos;
        // QGraphicsItem::mouseMoveEvent(event);
    }
    //    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override{
    //        event->accept();
    //    }
};

class BookmarksLine: public QGraphicsItem{
public:
    BookmarksLine(const Palette& plt, QPointer<BookmarkManager> bmkMngr, RenderInfo& ri, const QSharedPointer<TimeConvertor>& c):
        m_plt(plt), m_bmkMngr(bmkMngr), m_ri(ri), m_tc(c){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);
        ri.bookmarksBottomY = ri.bookmarksTopY + ri.bookmarksHeight;
    }
private:
    const Palette& m_plt;
    QPointer<BookmarkManager> m_bmkMngr;
    const RenderInfo& m_ri;
    QSharedPointer<TimeConvertor> m_tc;
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
        QRectF bkRect(m_tc->getPxPosFromMsec(b.start), m_ri.bookmarksTopY,
                      m_tc->getPxPosFromMsec(b.end), m_ri.bookmarksBottomY);
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
        QPoint posTopLeft{m_tc->getPxPosFromMsec(mb.start), m_ri.bookmarksTopY};

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
