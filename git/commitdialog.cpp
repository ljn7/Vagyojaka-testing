#include "commitdialog.h"
#include "qformlayout.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include <iostream>

CommitDialog::CommitDialog(QWidget *parent, QString prompt, bool commitMessageFlag)
    : QDialog(parent)
    , prompt(prompt)
    , commitMessageFlag(commitMessageFlag) {
    setupUI();
}

void CommitDialog::acceptInput() {
    accept();
}

void CommitDialog::setupUI() {
    QFormLayout *layout = new QFormLayout(this);
    usernameLineEdit = new QLineEdit(this);
    emailLineEdit = new QLineEdit(this);
    commitMessageLineEdit = new QLineEdit(this);

    layout->addRow((prompt + " Username: "), usernameLineEdit);
    layout->addRow((prompt + " Email: "), emailLineEdit);

    if (commitMessageFlag)
        layout->addRow("Commit Message: ", commitMessageLineEdit);

    okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &CommitDialog::acceptInput);

    layout->addWidget(okButton);

    setLayout(layout);
}

QString CommitDialog::getUsername() const {
    return this->usernameLineEdit->text();
}

QString CommitDialog::getEmail() const {
    return this->emailLineEdit->text();
}

QString CommitDialog::getCommitMessage() const {
    return this->commitMessageLineEdit->text();
}


CommitDialog::~CommitDialog() {
    delete usernameLineEdit;
    delete emailLineEdit;
    delete okButton;
    delete commitMessageLineEdit;
}

