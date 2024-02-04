#ifndef GIT_H
#define GIT_H

#include "qstatusbar.h"
#include <QObject>
#include <QWidget>
#include <git2.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class Git;
}
QT_END_NAMESPACE

class Git : public QWidget
{
    Q_OBJECT
public:
    explicit Git(QWidget *parent = nullptr);

    void init();
    void add();
    void commit();
    void addRemoteUrl();
    void pull();
    void fetch();
    void refresh();
    void push();
    ~Git();


signals:

private:
    git_repository *repo = nullptr;
    QString repoPath;
    QStatusBar *statusBar = nullptr;
    Ui::Git *ui;
    bool doesRepositoryExist(const char* path);
};

#endif // GIT_H
