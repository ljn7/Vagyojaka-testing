#ifndef TRANSCRIPTGENERATOR_H
#define TRANSCRIPTGENERATOR_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include <QFileDialog>
#include <QStandardPaths>
class TranscriptGenerator : public QObject
{
    Q_OBJECT
        public:
                 explicit TranscriptGenerator(QObject *parent = nullptr,    QUrl *fileUr=NULL);
                TranscriptGenerator(QUrl *fileUr=NULL);
signals:

public slots:
    void Upload_and_generate_Transcript();

private:
    QUrl *fileUrl;

};

#endif // TRANSCRIPTGENERATOR_H
