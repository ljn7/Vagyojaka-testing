#ifndef CREDENTIALSDIALOG_H
#define CREDENTIALSDIALOG_H

#include "qdialog.h"
#include "qlineedit.h"
#include "qwidget.h"
class CredentialsDialog  : public QDialog {
    Q_OBJECT

public:
    CredentialsDialog(QWidget *parent = nullptr);
    QString getData() const;
    // QString getPassword() const;
    void setPrompt(QString prompt, bool password);

private slots:
    void acceptInput();


private:
    void setupUI();
    QLineEdit *dataLineEdit;
    QString prompt;
    bool password;
    // QLineEdit *passwordLineEdit;
};

#endif // CREDENTIALSDIALOG_H
