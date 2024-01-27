#include "about.h"
#include "ui_about.h"
#include "qdesktopservices.h"

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    QDialog::setWindowTitle("About");

}

About::~About()
{
    delete ui;
}

void About::on_openInBrowser_clicked()
{
    QDesktopServices::openUrl(QUrl("https://va-gyojaka.vercel.app/"));
}

