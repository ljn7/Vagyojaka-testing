#ifndef REMOTENAMEDIALOG_H
#define REMOTENAMEDIALOG_H

#include "qlineedit.h"
#include <QDialog>

class RemoteNameDialog : public QDialog
{
    Q_OBJECT
public:
    RemoteNameDialog(QWidget *parent);
    QString getRemoteName() const;

private slots:
    void acceptInput();

private:
    void setupUI();
    QLineEdit *remoteNameLineEdit;
    QPushButton *okButton;
};

#endif // REMOTENAMEDIALOG_H
