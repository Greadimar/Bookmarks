#ifndef BOOKMARKSLINE_H
#define BOOKMARKSLINE_H

#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QPointer>
#include <QGraphicsSceneHoverEvent>
#include "timeaxis.h"
#include "bookmarkmanager.h"
#include "renderinfo.h"
#include "palette.h"
#include "bookmarkmanager.h"


class BookmarksLine: public QGraphicsItem{
public:
    BookmarksLine(const Palette& plt, QPointer<BookmarkManager> bmkMngr, RenderInfo& ri, const QSharedPointer<TimeAxis>& t);
    bool getMouseOnMultiBk() const;

    QPointF getPosForExtraTable() const;

private:
    const Palette& m_plt;
    QPointer<BookmarkManager> m_bmkMngr;
    const RenderInfo& m_ri;
    QSharedPointer<TimeAxis> m_ta;
    QMap<int, MultiBookmark> curBkMap;
    QFont font{"Times", 10};
    const int multiBoookmarkWidth = 100;
    const int borderWidth = 2;

    QPointF curMousePos{0,0};
    QPointF posForExtraTable{0, 0};
    bool mouseOnMultiBk{false};
    bool isHovered{false};

    QRectF boundingRect() const override {
         QRectF r{QPointF{0, static_cast<qreal>(m_ri.bookmarksTopY)},
         QPointF{static_cast<qreal>(m_ta->rulerWidth), static_cast<float>(m_ri.bookmarksBottomY)}};
        return r;
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *st, QWidget *) override;


    void paintMbk(const MultiBookmark& mb, QPainter* p, const QStyleOptionGraphicsItem *st);

    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override{
         curMousePos = mapToScene(event->pos());
         qDebug() << "h" << curMousePos;

    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override{
        isHovered = false;
        QGraphicsItem::hoverEnterEvent(event);
    }
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override{
        isHovered = true;
        QGraphicsItem::hoverEnterEvent(event);
    }

};

#endif // BOOKMARKSLINE_H
