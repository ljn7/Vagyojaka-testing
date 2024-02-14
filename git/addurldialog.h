#ifndef ADDURLDIALOG_H
#define ADDURLDIALOG_H

#include "qlineedit.h"
#include <QDialog>

class AddUrlDialog : public QDialog
{
    Q_OBJECT
public:
    AddUrlDialog(QWidget *parent);
    QString getRemoteName() const;
    QString getRemoteURL() const;

private slots:
    void acceptInput();

private:
    void setupUI();
    QLineEdit *remoteNameLineEdit;
    QLineEdit *remoteURLLineEdit;
    QPushButton *okButton;
};

#endif // ADDURLDIALOG_H
