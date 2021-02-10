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
#include "timeconvertor.h"
#include <QPointer>
namespace Ui {
class SceneWt;
}


inline QPointF randomPosition() {
    static std::random_device dev;
    static std::default_random_engine eng(dev());
    static std::uniform_real_distribution<double> posDis(-100., 100.); // NM
    return {posDis(eng), posDis(eng)};
}

class EmptyItem : public QGraphicsItem {
public:
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
};

class SceneManager : public QObject{
    Q_OBJECT
    Q_PROPERTY(bool microGraticuleVisible READ microGraticuleVisible WRITE setMicroGraticuleVisible)
    QGraphicsScene m_scene;
    QPen m_targetPen{Qt::green, 1};
    EmptyItem m_target, m_center, m_macroGraticule, m_microGraticule; //m_center is ok
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Resize
                && qobject_cast<QGraphicsView*>(watched))
            emit viewResized();
        return QObject::eventFilter(watched, event);
    }
signals:
    void viewResized();
public slots:
    void recenterView(){
        //auto range = combo.currentText().toDouble();
       // fitInView(-range, -range, 2.*range, 2.*range, Qt::KeepAspectRatio);
       // mgr.setMicroGraticuleVisible(range <= 20.);
    }
public:
    SceneManager() {
        m_scene.addItem(&m_center);
        m_scene.addItem(&m_macroGraticule);
        m_scene.addItem(&m_microGraticule);
        m_scene.addItem(&m_target);
        m_targetPen.setCosmetic(true);
        addGraticules();
    }
    void monitor(QGraphicsView *view) { view->installEventFilter(this); }
    QGraphicsScene * scene() { return &m_scene; }
    Q_SLOT void setMicroGraticuleVisible(bool vis) { m_microGraticule.setVisible(vis); }
    bool microGraticuleVisible() const { return m_microGraticule.isVisible(); }
    void newTargets(int count = 200) {
        qDeleteAll(m_target.childItems());
        for (int i = 0; i < count; ++i) {
            auto target = new QGraphicsEllipseItem(-1.5, -1.5, 3., 3., &m_target);
            target->setPos(randomPosition());
            target->setPen(m_targetPen);
            target->setBrush(m_targetPen.color());
            target->setFlags(QGraphicsItem::ItemIgnoresTransformations);
        }
    }
    void addGraticules() {
        QPen pen{Qt::white, 1};
        pen.setCosmetic(true);
        auto center = {QLineF{-5.,0.,5.,0.}, QLineF{0.,-5.,0.,5.}};
        for (auto l : center) {
            auto c = new QGraphicsLineItem{l, &m_center};
            c->setFlags(QGraphicsItem::ItemIgnoresTransformations);
            c->setPen(pen);
        }
        for (auto range = 10.; range < 101.; range += 10.) {
            auto circle = new QGraphicsEllipseItem(0.-range, 0.-range, 2.*range, 2.*range, &m_macroGraticule);
            circle->setPen(pen);
        }
        pen = QPen{Qt::white, 1, Qt::DashLine};
        pen.setCosmetic(true);
        for (auto range = 2.5; range < 9.9; range += 2.5) {
            auto circle = new QGraphicsEllipseItem(0.-range, 0.-range, 2.*range, 2.*range, &m_microGraticule);
            circle->setPen(pen);
        }
    }


};

class TestItem: public QGraphicsItem{

    QRectF boundingRect() const override{
        return QRectF(0, 0, 20, 20);
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget* widget){
        painter->drawRoundedRect(0,0,20,20, 5, 5);
    }
};

class RuleView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit RuleView(QPointer<BookmarkManager> mngr, QSharedPointer<TimeConvertor> timec,
                     QWidget *parent = nullptr): m_bkmngr(mngr), m_timeconv(timec){
        setRenderHints(QPainter::SmoothPixmapTransform| QPainter::TextAntialiasing);
       // setCacheMode(QGraphicsView::CacheNone); // change ??
        this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        setFrameStyle(0); // to remove borders between view and scene
        setSceneRect(0, 0, this->width(), height()); // check without

        // scene.setItemIndexMethod(QGraphicsScene::NoIndex); sometimes for dynamic scene should set no index
        setScene(&m_scene);
       // setBackgroundBrush(QBrush(QColor("#eaeaeaea")));
        m_scene.setBackgroundBrush(QBrush(QColor("#808080")));
      //  m_scene.setB
        initElements();
        renderTimer();
//        auto t = new QTimer();
//        connect(t, &QTimer::timeout, this, [=](){
//            scene()->advance();
//        });
//        t->start(50);

    }
    ~RuleView(){}

private:


    Ui::SceneWt *ui;
    Scene m_scene;
    QGraphicsPixmapItem *pixmapItem;
    QPixmap *pixmap;
    //
    QPointer<BookmarkManager> m_bkmngr;
    QSharedPointer<TimeConvertor> m_timeconv;

    //items
    Ruler* m_ruler;
    BookmarksLine* m_line;

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

    void renderTimer(){
        auto cur = std::chrono::system_clock::now();
        auto sinceLastRender = cur - renderInfo.lastRender;
        if (sinceLastRender > msecs(renderInfo.renderStep)) sinceLastRender = msecs(0);
        auto&& nextRender = renderInfo.renderStep - sinceLastRender;
        renderInfo.lastRender = cur;
        QTimer::singleShot(std::chrono::duration_cast<msecs>(nextRender), this, [=](){
            m_ruler->update();
            m_line->update();
            renderTimer();
        });
    }
private:
    void resizeEvent(QResizeEvent *event) override{
        QTimer::singleShot(100, this, [=](){
    //       initElements();
            qDebug() << "w" << scene()->width();
        });
        QGraphicsView::resizeEvent(event);
    }
};

#endif // SCENEWT_H
