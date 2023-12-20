#pragma once

#include "qdialog.h"


class MediaSplitter : public QDialog
{
    Q_OBJECT
public:
    explicit MediaSplitter(QWidget* parent = nullptr);
    explicit MediaSplitter(QWidget* parent, QString mediaFileName,
                           QList<QTime> timeStamps);

    void splitMedia();

private:
    bool splitMediaUtil(uint64_t startSeconds, uint64_t endSeconds);
    QString mediaFileName;
    QList<QTime> timeStamps;
    uint64_t counter;
};

