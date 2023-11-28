#ifndef EQUATIONEDITOR_H
#define EQUATIONEDITOR_H

#include <QMainWindow>

namespace Ui {
class equationeditor;
}

class equationeditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit equationeditor(QWidget *parent = nullptr);
    ~equationeditor();

private:
    Ui::equationeditor *ui;
};

#endif // EQUATIONEDITOR_H
