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

struct ViewPositions{
    int rulerTopY{0};
    int rulerBottomY{100};
    const int bookmarksTopMargin = 10;
    const int leftMargin = 5;
    const int rightMargin = 5;
    int bookmarksTopY{10};
    int bookmarksBottomY{100};
    int bookmarksHeight = 90;
    int rulerWidth = 100;
    float zoomRatio = 1; //1 - 24h, 2 = 1h
    int dragPosition = 0;
};

class Ruler: public QGraphicsItem{
public:
    Ruler(const Palette& plt, ViewPositions& vp, const QSharedPointer<TimeConvertor>& c): m_plt(plt), m_vp(vp), m_tc(c){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);

        hour1xLabelsOffsetToCenter = fm.horizontalAdvance("1h")/2;
        hour2xLabelsOffsetToCenter = fm.horizontalAdvance("24h")/2;
        hourLabelsBottomPos = linesHeight + fm.height() + hourLabelsTopMargin;
        vp.rulerBottomY = hourLabelsBottomPos;
        vp.bookmarksTopY = vp.rulerBottomY + vp.bookmarksTopMargin;
    }
private:
    const Palette& m_plt;
    ViewPositions& m_vp;
    QSharedPointer<TimeConvertor> m_tc;
    QFont font{"Times", 10};
    double linesHeight = 30;
    const float hourLabelsTopMargin = 5;
    float hourLabelsBottomPos;
    float hour1xLabelsOffsetToCenter = 0;
    float hour2xLabelsOffsetToCenter = 0;


    QRectF boundingRect() const override {
        return QRectF{QPointF{0,0}, QPointF{static_cast<float>(m_vp.rulerWidth), static_cast<float>(m_vp.rulerBottomY)}};
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
      //  p->save();
        QPointF curPos = scenePos();
        int viewWidth = scene()->views().first()->width();
        m_vp.rulerWidth = viewWidth - m_vp.leftMargin - m_vp.rightMargin;
        m_tc->rulerWidth.store(m_vp.rulerWidth); //std::rel_ack?
        m_tc->hourWidthInPx = (m_vp.rulerWidth/(-23 * m_vp.zoomRatio + 47)); //y = 47 - 23x
        //drawing ruler
        p->setPen(m_plt.rulerBackground);
        p->drawRect(0, 0, viewWidth,  hourLabelsBottomPos);
        p->setPen(QColor{"#ed9121"});
        p->drawRect(m_vp.leftMargin, 0, m_vp.rulerWidth,  hourLabelsBottomPos);

        //drawing lines and labels
        p->setPen(QPen(m_plt.labels, 2));
        p->setFont(font);

        int time{0};

        float i = m_vp.leftMargin;
        for (; time < 10; time++){
            if (i > m_vp.leftMargin && i < m_vp.rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour1xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_tc->hourWidthInPx;
        }
        for (; time <= 24; time++){
            if (i > m_vp.leftMargin && i < m_vp.rulerWidth){
                p->drawLine({curPos.rx() + i, curPos.ry()}, QPointF{curPos.rx() + i, curPos.ry() + linesHeight});
                p->drawText(QPointF{i - hour2xLabelsOffsetToCenter, hourLabelsBottomPos}, QString("%1h").arg(time));
            }
            i += m_tc->hourWidthInPx;
        }
       // p->restore();
    }

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override{
        int delta = event->delta();
        static const float wheelScrollRatio = 0.001;
        float targetZoomRatio{m_vp.zoomRatio};
        targetZoomRatio += delta * wheelScrollRatio;
        if (targetZoomRatio < 1) targetZoomRatio = 1;
        else if (targetZoomRatio > 2) targetZoomRatio = 2;
        m_vp.zoomRatio = targetZoomRatio;
        return;
    }
};

class BookmarksLine: public QGraphicsItem{
public:
    BookmarksLine(const Palette& plt, QPointer<BookmarkManager> bmkMngr, ViewPositions& vp, const QSharedPointer<TimeConvertor>& c): m_plt(plt), m_bmkMngr(bmkMngr), m_vp(vp), m_tc(c){
        this->font.setStyleStrategy(QFont::ForceOutline);
        QFontMetrics fm(font);
        vp.bookmarksBottomY = vp.bookmarksTopY + vp.bookmarksHeight;
    }
private:
    const Palette& m_plt;
    QPointer<BookmarkManager> m_bmkMngr;
    const ViewPositions& m_vp;
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
        int width = viewWidth - m_vp.leftMargin - m_vp.rightMargin;
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
        qDebug() << "paint: get bufPtr LOCK";
         // auto& displayBufPtr = m_bmkMngr->displayBuffer;
        m_bmkMngr->mutex.lock();
        auto&& vec = *m_bmkMngr->displayBuffer;

        for( auto& b: vec){
             if (b.count == 1) paintBk(b, p, st);
             else paintMbk(b, p, st);
        }
        m_bmkMngr->mutex.unlock();
        m_bmkMngr->displayBufIsBusy.store(false);
                qDebug() << "paint: UNLOCK";
    }

    void paintBk(const Bookmark& b, QPainter* p, const QStyleOptionGraphicsItem *st){
         p->setPen(QPen(m_plt.singleBookmarksBorders, 2));
         QRectF bkRect(m_tc->getPxPosFromMsec(b.start), m_vp.bookmarksTopY,
                m_tc->getPxPosFromMsec(b.end), m_vp.bookmarksBottomY);
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
         QPoint posTopLeft{m_tc->getPxPosFromMsec(mb.start), m_vp.bookmarksTopY};

         int w = posTopLeft.rx() + multiBoookmarkWidth - borderWidth;
         int h = m_vp.bookmarksHeight;
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
