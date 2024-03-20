#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTime>
#include <QSettings>

//New Include
#include <QAudioOutput>

class MediaPlayer : public QMediaPlayer
{
    Q_OBJECT
public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    QTime elapsedTime();
    QTime durationTime();
    void setPositionToTime(const QTime& time);
    QString getMediaFileName();
    QString getPositionInfo();
    QStringList supportedFormats;
    // QAudioOutput *m_audioOutput;
    void loadMediaFromUrl(QUrl *fileUrl);

public slots:
    void open();
    void seek(int seconds);
    void togglePlayback();
    // void setVolume(int volume);
    // void setMuted(bool muted);

signals:
    void message(QString text, int timeout = 5000);

private:
    static QTime getTimeFromPosition(const qint64& position);
    QString m_mediaFileName;
    QSettings* settings;

};
