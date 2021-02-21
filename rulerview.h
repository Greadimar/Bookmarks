#ifndef SCENEWT_H
#define SCENEWT_H

#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <random>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPainter>
#include <QTimer>
#include "ruler.h"
#include "bookmarkmanager.h"
#include "timeaxis.h"
#include "bookmarksline.h"
#include <QPointer>
#include <QGraphicsProxyWidget>
#include <QTableView>
#include <QGraphicsScene>
#include "extratablemodel.h"
class RuleView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit RuleView(QPointer<BookmarkManager> mngr, QPointer<TimeAxis> timea,
                     QWidget *parent = nullptr): QGraphicsView(parent), m_bkmngr(mngr), m_ta(timea){
        setRenderHints(QPainter::SmoothPixmapTransform| QPainter::TextAntialiasing);
       // setCacheMode(QGraphicsView::CacheNone); // change ??
        this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        setFrameStyle(0); // to remove borders between view and scene
        setSceneRect(0, 0, this->width(), height()); // check without

        // scene.setItemIndexMethod(QGraphicsScene::NoIndex); sometimes for dynamic scene should set no index
        setScene(&m_scene);

        m_scene.setBackgroundBrush(QBrush(QColor("#989898")));

        QTimer::singleShot(10, this, [=](){
            this->setMinimumSize(sizeHint());
             initElements();

             renderTimer();
        });
    }
    ~RuleView(){}

    Ruler *getRuler() const;

    ExtraTableModel *getModel() const;

private:

    QGraphicsScene m_scene;
    QGraphicsPixmapItem *pixmapItem;
    QPixmap *pixmap;
    //
    QPointer<BookmarkManager> m_bkmngr;
    QPointer<TimeAxis> m_ta;
    ExtraTableModel* model;
    QTableView* extraTable;
    QGraphicsProxyWidget* proxyWtTable;

    //items
    Ruler* m_ruler;
    BookmarksLine* m_line;
    bool tableIsHovered{false};


    //operating structs

    RenderInfo renderInfo;
    Palette plt;



    //pens
    QPen blackPen{Qt::black, 1};
    QPen boldBlackPen{Qt::black, 3};

    int lineHeight{30};

    //coords
    QPointF get12hLineUp(){
        return {this->width()/2. ,0};
    }
    QPointF get12hLineDown(){
        return {this->width()/2., static_cast<double>(lineHeight)};
    }
    void initElements();

    void renderTimer();
private:
    void toRender();
    void resizeEvent(QResizeEvent *event) override{

        QGraphicsView::resizeEvent(event);
    }

};

#endif // SCENEWT_H
