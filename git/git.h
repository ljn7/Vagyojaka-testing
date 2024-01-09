#ifndef GIT_H
#define GIT_H

#include <QObject>
#include <QWidget>
#include <git2.h>

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

    ~Git();


signals:

private:
    git_repository *repo = nullptr;
    QString repoPath;
    bool doesRepositoryExist(const char *path);
};

#endif // GIT_H
