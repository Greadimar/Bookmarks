#include "bookmarksline.h"
#include <QFontMetrics>
#include <QStyleOptionGraphicsItem>
BookmarksLine::BookmarksLine(const Palette &plt, QPointer<BookmarkManager> bmkMngr, RenderInfo &ri, const QPointer<TimeAxis> &t):
    m_plt(plt), m_bmkMngr(bmkMngr), m_ri(ri), m_ta(t){
    this->font.setStyleStrategy(QFont::ForceOutline);
    QFontMetrics fm(font);
    ri.bookmarksBottomY = ri.bookmarksTopY + ri.bookmarksHeight;
    setAcceptHoverEvents(true);
    posForExtraTable.setY(ri.bookmarksBottomY + ri.extraTableTopMargin);
}

bool BookmarksLine::getMouseOnMultiBk() const
{
    return mouseOnMultiBk;
}

QPointF BookmarksLine::getPosForExtraTable() const
{
    return posForExtraTable;
}

void BookmarksLine::paint(QPainter *p, const QStyleOptionGraphicsItem *st, QWidget *) {
    if (m_bmkMngr->isRunning){
        //qDebug() << "thrDisp";
        //  QMap<int, MultiBookmark> updMap;
        auto bks = m_bmkMngr->getToDisplay();
        bool foundHover{false};
        mouseOnMultiBk = false;
        for (auto& mbk : bks){
            paintMbk(mbk, p, st);
            int s = m_ta->pxPosFromMsec(mbk.start);
            int e = m_ta->pxPosFromMsec(mbk.end);

            if (!foundHover){
                if (curMousePos.rx() >  s && curMousePos.rx() < e){
                    if (curMousePos.ry() < m_ri.bookmarksTopY || curMousePos.ry() > m_ri.bookmarksBottomY) continue;
                    if (s + m_ri.tableSize.width() > m_ta->rulerWidth){
                        s = e - m_ri.tableSize.width();
                    }
                    posForExtraTable.setX(s);
                    foundHover = true;
                    mouseOnMultiBk = true & isHovered;
                    m_bmkMngr->extraTableMin = mbk.start;
                    m_bmkMngr->extraTableMax = mbk.end;

                }
            }
            // updMap.insert(mbk.start, mbk);
        }
    }
}

void BookmarksLine::paintMbk(const MultiBookmark &mb, QPainter *p, const QStyleOptionGraphicsItem *st){
    if (mb.count != 1) p->setPen(QPen(m_plt.multiBookmarksBorders, 2));
    else p->setPen(QPen(m_plt.singleBookmarksBorders, borderWidth));
    QPoint posTopLeft{m_ta->pxPosFromMsec(mb.start), m_ri.bookmarksTopY};

    int w = m_ta->pxPosFromMsec(mb.end) - posTopLeft.rx() - borderWidth;
    int h = m_ri.bookmarksHeight;
    // qDebug() << "painting bk: " << posTopLeft << w << h;
    QRectF bkRect(posTopLeft.rx(), posTopLeft.ry(), w, h);
    QPainterPath pp;
    pp.addRoundedRect(bkRect, 5, 5);
    if (mb.count == 1) p->fillPath(pp, QBrush(m_plt.singleBookmarksBackground));
     else p->fillPath(pp, QBrush(m_plt.multiBookmarksBackground));
    QString name = mb.name;
    p->drawRect(bkRect);
    if (mb.count == 1){
        if (st->fontMetrics.horizontalAdvance(name) > w - 4*borderWidth){
            name = "...";
            if (st->fontMetrics.horizontalAdvance(name) > w - 4*borderWidth){
                name = "";
            }
        }
    }
    else{
        name = QString::number(mb.count);
    }

    QPointF namePos{bkRect.x() + bkRect.width()/2 - st->fontMetrics.horizontalAdvance(name)/2,
                bkRect.y() + 2*bkRect.height()/3};

    if (st->fontMetrics.horizontalAdvance(name) > (w - 4*borderWidth)){
        name = "...";
        if (st->fontMetrics.horizontalAdvance(name) > w - 4*borderWidth){
            name = "";
        }
    }
    p->drawText(namePos, name);

}
