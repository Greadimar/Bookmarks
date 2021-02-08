#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QOpenGLWidget>
#include <QSurfaceFormat>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
//    auto lo = new QGridLayout();
//    lo->addWidget(&m_view);
//    this->setLayout(lo);



    timeconvertor = QSharedPointer<TimeConvertor>::create();
    bookmarkMngr = new BookmarkManager(timeconvertor);

    //backend
    bookmngrThr = new QThread();
    connect(bookmngrThr, &QThread::finished, bookmarkMngr, &QObject::deleteLater);
    connect(bookmngrThr, &QThread::finished, bookmngrThr, &QObject::deleteLater);
    bookmarkMngr->moveToThread(bookmngrThr);
    bookmngrThr->start();
    QMetaObject::invokeMethod(bookmarkMngr, "start");
   // bookmarkMngr.start();

    //QMetaObject::invokeMethod(bookmarkMngr.data(), "start");
    connect(&bookmarkMngr->getFileWorker(), &QFileBuffer::sendPrg, this, [=](int val){
        ui->progressBar->setValue(val);
    });

    //view

    view = new RuleView(bookmarkMngr, timeconvertor);

    ui->lo->addWidget(view);

    auto menu = new QMenu("Отрисовка");
    this->menuBar()->addMenu(menu);
    auto drawOGl = new QAction("OpenGL");
    connect(drawOGl, &QAction::triggered, this, &MainWindow::drawOpenGl);
    menu->addAction(drawOGl);

    auto generateAction = new QAction("Сгенерировать");
    this->menuBar()->addAction(generateAction);
    connect(generateAction, &QAction::triggered, this, [=](){
        bookmarkMngr->stop();
        bool isGenerated = false;
        QMetaObject::invokeMethod(bookmarkMngr.data(), "generateBookmarks", Q_ARG(int, 100000));
       // bookmarkMngr->start();
    });

  //  menu->addA
}

MainWindow::~MainWindow()
{
    bookmarkMngr->stop();
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

