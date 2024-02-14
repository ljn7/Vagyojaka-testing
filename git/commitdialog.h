#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

#include "qlineedit.h"
#include <QDialog>

class CommitDialog : public QDialog
{
    Q_OBJECT

public:
    CommitDialog(QWidget *parent = nullptr, QString prompt = "", bool commitMessageFlag = false);
    void setPrompt(QString prompt, bool password);
    QString getUsername() const;
    QString getEmail() const;
    QString getCommitMessage() const;
    ~CommitDialog();
    void setPrompt(QString prompt);

private slots:
    void acceptInput();

private:
    void setupUI();
    QLineEdit *usernameLineEdit;
    QLineEdit *emailLineEdit;
    QLineEdit *commitMessageLineEdit;
    QString prompt;
    QPushButton *okButton;
    bool commitMessageFlag;
};

#endif // COMMITDIALOG_H
