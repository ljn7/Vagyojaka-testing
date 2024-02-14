#include "addurldialog.h"
#include "qformlayout.h"
#include "qpushbutton.h"

AddUrlDialog::AddUrlDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();

}

void AddUrlDialog::setupUI() {
    setWindowTitle("Set/Modifiy Remote URL");
    QFormLayout *layout = new QFormLayout(this);
    remoteNameLineEdit = new QLineEdit(this);
    remoteURLLineEdit = new QLineEdit(this);
    layout->addRow("Remote Name: ", remoteNameLineEdit);
    layout->addRow("Remote URL: ", remoteURLLineEdit);

    okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &AddUrlDialog::acceptInput);

    layout->addWidget(okButton);

    setLayout(layout);
}

QString AddUrlDialog::getRemoteName() const {
    return this->remoteNameLineEdit->text();
}

QString AddUrlDialog::getRemoteURL() const {
    return this->remoteURLLineEdit->text();
}

void AddUrlDialog::acceptInput() {
    accept();
}
