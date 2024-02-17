#include "remotenamedialog.h"
#include "qformlayout.h"
#include "qpushbutton.h"

RemoteNameDialog::RemoteNameDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void RemoteNameDialog::setupUI() {
    setWindowTitle("Remote Name");
    QFormLayout *layout = new QFormLayout(this);
    remoteNameLineEdit = new QLineEdit(this);
    layout->addRow("Remote Name: ", remoteNameLineEdit);

    okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &RemoteNameDialog::acceptInput);

    layout->addWidget(okButton);

    setLayout(layout);

}

void RemoteNameDialog::acceptInput() {
    accept();
}

QString RemoteNameDialog::getRemoteName() const {
    return this->remoteNameLineEdit->text();
}
