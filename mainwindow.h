#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "rulerview.h"
#include <QMainWindow>
#include <QPointer>
#include <QLabel>

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

    enum class State{
        generating,
        idle
    };
    State state{State::idle};
    int curSize{0};
    QLabel* lblStatus;
    QPointer<TimeAxis> timeAxis;
    QPointer<BookmarkManager> bookmarkMngr;
    QPointer<RuleView> view;


    QThread* bookmngrThr;
    void drawOpenGl();
};
#endif // MAINWINDOW_H
