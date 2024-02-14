#include "credentialsdialog.h"
#include "qformlayout.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include <git2.h>

CredentialsDialog::CredentialsDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Git Credentials");
}




void CredentialsDialog::setupUI() {
    QFormLayout *layout = new QFormLayout(this);

    dataLineEdit = new QLineEdit(this);
    if (password)
        dataLineEdit->setEchoMode(QLineEdit::Password);
    layout->addRow(prompt + ": ", dataLineEdit);

    QPushButton *okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &CredentialsDialog::acceptInput);

    layout->addWidget(okButton);

    setLayout(layout);
}

void CredentialsDialog::acceptInput() {
    accept();
}

QString CredentialsDialog::getData() const {
    return dataLineEdit->text();
}

void CredentialsDialog::setPrompt(QString prompt, bool password = false) {
    this->password = password;
    this->prompt = prompt;
    setupUI();
}
