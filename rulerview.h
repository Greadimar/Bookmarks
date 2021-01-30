#ifndef SCENEWT_H
#define SCENEWT_H

#include <QWidget>

namespace Ui {
class SceneWt;
}

class SceneWt : public QWidget
{
    Q_OBJECT

public:
    explicit SceneWt(QWidget *parent = nullptr);
    ~SceneWt();

private:
    Ui::SceneWt *ui;
};

#endif // SCENEWT_H
