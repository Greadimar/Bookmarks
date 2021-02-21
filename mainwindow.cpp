#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
//    auto lo = new QGridLayout();
//    lo->addWidget(&m_view);
//    this->setLayout(lo);



    timeAxis = new TimeAxis;
    bookmarkMngr = new BookmarkManager(timeAxis);

    //backend
    bookmngrThr = new QThread();

    connect(bookmngrThr, &QThread::finished, bookmarkMngr, &QObject::deleteLater);
    connect(bookmngrThr, &QThread::finished, bookmngrThr, &QObject::deleteLater);
    bookmarkMngr->moveToThread(bookmngrThr);
    bookmngrThr->start();
        bookmngrThr->setPriority(QThread::HighPriority);

    connect(bookmarkMngr, &BookmarkManager::sendPrg, this, [=](int val){
        ui->progressBar->setValue(val);
    });

    //view

    view = new RuleView(bookmarkMngr, timeAxis);

    ui->lo->addWidget(view);

    auto drawMenu = new QMenu("Отрисовка");
   // this->menuBar()->addMenu(menu);
    auto drawOGl = new QAction("OpenGL");
    connect(drawOGl, &QAction::triggered, this, &MainWindow::drawOpenGl);
    drawMenu->addAction(drawOGl);

    auto generateAction = new QAction("Сгенерировать");
    this->menuBar()->addAction(generateAction);

    connect(generateAction, &QAction::triggered, this, [=](){
        //bookmarkMngr->stop();
        QDialog d;
        d.setWindowTitle("Сгенерировать отметки");
        d.setMinimumWidth(400);
        QGridLayout* lo = new QGridLayout;
        QSpinBox* sb = new QSpinBox;
        sb->setRange(1, 1000000000);
        QLabel* lb = new QLabel("Количество: ");
        QPushButton* pb = new QPushButton("Сгенерировать");
        connect(pb, &QPushButton::clicked , this , [=, &d](){
            bookmarkMngr->stop();
            view->getModel()->clear();
            curSize = sb->value();
            state = State::generating;
            QMetaObject::invokeMethod(bookmarkMngr.data(), "generateBookmarks", Q_ARG(int, sb->value()));
            d.close();

        });

        lo->addWidget(lb, 0, 0);
        lo->addWidget(sb, 0, 1);
        lo->addWidget(pb, 1, 0, 1, 2);
        d.setLayout(lo);
        d.exec();


    });

    auto qAction = new QAction("Запрос");
  //  this->menuBar()->addAction(qAction);
    connect(qAction, &QAction::triggered, this, [=](){
       bookmarkMngr->stop();
       QMetaObject::invokeMethod(bookmarkMngr.data(), "testGet");


    });

    //create menu for animation
    auto menuSets = new QMenu("Настройки");
    auto menuAni = new QMenu("Анимации");
    auto actNoAni = new QAction("Без анимации");
    auto actSimpleAni = new QAction("Простая анимация");
    auto actInertion = new QAction("Инерции");

    actNoAni->setCheckable(true);
    actSimpleAni->setCheckable(true);
    actInertion->setCheckable(true);

    actNoAni->setChecked(true);

    connect(actNoAni, &QAction::triggered, this, [=](){
        view->getRuler()->setAnimation(Ruler::Animation::noAnimation);
        actNoAni->setChecked(true);
        actSimpleAni->setChecked(false);
        actInertion->setChecked(false);
    });
    connect(actSimpleAni, &QAction::triggered, this, [=](){
        view->getRuler()->setAnimation(Ruler::Animation::simpleAnimation);
        actNoAni->setChecked(false);
        actSimpleAni->setChecked(true);
        actInertion->setChecked(false);
    });

    connect(actInertion, &QAction::triggered, this, [=](){
        view->getRuler()->setAnimation(Ruler::Animation::animationWithInertion);
        actNoAni->setChecked(false);
        actSimpleAni->setChecked(false);
        actInertion->setChecked(true);
    });
    menuAni->addAction(actNoAni);
    menuAni->addAction(actSimpleAni);
    menuAni->addAction(actInertion);
    menuSets->addMenu(menuAni);
    menuBar()->addMenu(menuSets);
    menuSets->addMenu(drawMenu);


    //status timer
    QTimer* statusTimer = new QTimer(this);
    lblStatus = new QLabel(this);
    this->statusBar()->addPermanentWidget(lblStatus);
    connect(statusTimer, &QTimer::timeout, this, [=](){
       int c = bookmarkMngr->getSqlworker()->counter.load();
       if (c >= curSize) state = State::idle;

       lblStatus->setText(QString("сгенерировано: %1 из %2").arg(c).arg(curSize));
    });
    statusTimer->start(300);
  //  menu->addA
}

MainWindow::~MainWindow()
{
    bookmarkMngr->stop();
    bookmngrThr->quit();
    if (!bookmngrThr->wait(2000)){
        bookmngrThr->quit();
        bookmngrThr->wait(2000);
        qDebug() << Q_FUNC_INFO << "forced to stop thread";
    }
    delete ui;
}

void MainWindow::drawOpenGl()
{
    QOpenGLWidget *gl = new QOpenGLWidget();
    QSurfaceFormat format;
    format.setSamples(4);

    gl->setFormat(format);
    view->setViewport(gl);
          view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

