#include "scenewt.h"
#include "ui_scenewt.h"

SceneWt::SceneWt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneWt)
{
    ui->setupUi(this);
}

SceneWt::~SceneWt()
{
    delete ui;
}
