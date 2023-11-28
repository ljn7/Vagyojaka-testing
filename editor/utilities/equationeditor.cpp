#include "equationeditor.h"
#include "ui_equationeditor.h"

equationeditor::equationeditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::equationeditor)
{
    ui->setupUi(this);
}

equationeditor::~equationeditor()
{
    delete ui;
}
