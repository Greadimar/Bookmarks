#include "ruler.h"


Ruler::Ruler(const Palette &plt, RenderInfo &ri, const QSharedPointer<TimeAxis> &c): m_plt(plt), m_ri(ri), m_ta(c){
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

void Ruler::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
    //  p->save();
    QPointF curPos = scenePos();
    int viewWidth = scene()->views().first()->width();
    int rulerWidth = viewWidth;
    float resizeSinceLastRender = rulerWidth / static_cast<float>(m_ta->rulerWidth);
    m_ta->stepInPx = m_ta->stepInPx * resizeSinceLastRender;
    m_ta->hourWidthInPx = m_ta->hourWidthInPx * resizeSinceLastRender;
    m_ta->dayWidthInPx = m_ta->hourWidthInPx * 24;
    m_ta->rulerWidth.store(rulerWidth);

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


void Ruler::zoomToCenter(float targetZoomRatio){
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



BookmarksLine::BookmarksLine(const Palette &plt, QPointer<BookmarkManager> bmkMngr, RenderInfo &ri, const QSharedPointer<TimeAxis> &t):
    m_plt(plt), m_bmkMngr(bmkMngr), m_ri(ri), m_ta(t){
    this->font.setStyleStrategy(QFont::ForceOutline);
    QFontMetrics fm(font);
    ri.bookmarksBottomY = ri.bookmarksTopY + ri.bookmarksHeight;
    setAcceptHoverEvents(true);
    posForExtraTable.setY(ri.bookmarksBottomY);
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
        for (auto& mbk : bks){
            paintMbk(mbk, p, st);
            int s = m_ta->pxPosFromMsec(mbk.start);
            int e = m_ta->pxPosFromMsec(mbk.end);
            if (!foundHover){
                if (curMousePos.rx() >  s && curMousePos.rx() < e){
                    if (curMousePos.ry() > m_ri.bookmarksTopY && curMousePos.ry() < m_ri.bookmarksBottomY) continue;
                    posForExtraTable.setX(s);
                    foundHover = true;
                    mouseOnMultiBk = true;
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
    QRectF bkRect(posTopLeft.rx(), posTopLeft.ry(), w, h - posTopLeft.ry());
    QPainterPath pp;
    pp.addRoundedRect(bkRect, 5, 5);
    // if (mb.count == 1) p->fillPath(pp, QBrush(m_plt.singleBookmarksBackground));
    // else p->fillPath(pp, QBrush(m_plt.multiBookmarksBackground))
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
