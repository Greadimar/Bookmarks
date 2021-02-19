#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "rulerview.h"
#include <QMainWindow>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QPointer<TimeAxis> timeAxis;
    QPointer<BookmarkManager> bookmarkMngr;
    QPointer<RuleView> view;


    QThread* bookmngrThr;
    void drawOpenGl();
};
#endif // MAINWINDOW_H
