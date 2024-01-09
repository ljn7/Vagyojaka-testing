#include "git.h"
#include <git2.h>
#include <qdir.h>

Git::Git(QWidget *parent)
    : QWidget{parent}
{
    git_libgit2_init();
    repoPath = QDir::currentPath() + "/Splitted-Media";
    init();
}

void Git::init()
{
    QDir repoDir(repoPath);
    if(!repoDir.exists()) {
        qDebug() << "Initialing" << Qt::endl;
        repoDir.mkpath(repoPath);
    }

    if (!doesRepositoryExist(repoPath.toUtf8())) {

        int error = git_repository_init(&repo, repoPath.toUtf8(), 0);
        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error initializing repository:" << error << e->message;
            return;
        }

        qDebug() << "Repository initialized at:" << repoPath;

    } else {
        qDebug() << "Repository already exists at:" << repoPath << Qt::endl;
        qDebug() << "Opening Repository\n";

        int error = git_repository_open(&repo, repoPath.toUtf8());
        if (error < 0) {
            const git_error* e = giterr_last();
            qCritical() << "Error opening repository:" << error << e->message;
            git_libgit2_shutdown();
            return;
        }
        qDebug() << "Opening Repository was successful\n";
    }


}

void Git::add()
{

}

void Git::commit()
{

}

void Git::addRemoteUrl()
{

}

void Git::pull()
{

}

void Git::fetch()
{

}

void Git::refresh()
{

}

bool Git::doesRepositoryExist(const char* path) {
    int error = git_repository_open(&repo, path);
    if (error == 0) {
        // Repository exists
        git_repository_free(repo);
        return true;
    }
    return false;
}
Git::~Git() {

    git_repository_free(repo);
    git_libgit2_shutdown();
}
