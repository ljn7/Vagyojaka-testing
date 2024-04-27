#ifndef AUDIOPLAYERWIDGET_H
#define AUDIOPLAYERWIDGET_H

#include "qabstractbutton.h"
#include "qcombobox.h"
#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QLabel>
#include <QTime>

class AudioPlayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit AudioPlayerWidget(const QString &audioFilePath, QWidget *parent = nullptr);
    ~AudioPlayerWidget();

signals:
    void changeRate(qreal rate);
    void play();
    void pause();
    void stop();


private:
    qreal playbackRate() const;
    void togglePlayback();
    void setPosition(int position);
    void positionChanged(qint64 progress);
    void durationChanged(qint64 duration);
    // bool isMuted() const;
    void updateDurationInfo(qint64 currentInfo);
    void updateRate();
    void seek(int mseconds);
    void setState(QMediaPlayer::PlaybackState state);
    void playClicked();
    QMediaPlayer::PlaybackState state() const;
    // void muteClicked();
    // void setMuted(bool muted);


    QString m_audioFilePath;
    QMediaPlayer *m_mediaPlayer;
    QAbstractButton *m_playButton;
    QAbstractButton *m_stopButton;
    QAbstractButton *changeAudioButton;
    QAbstractSlider *m_slider;
    // QAbstractButton *m_muteButton;
    qint64 m_duration = 0;
    QLabel *m_labelDuration;
    QAudioOutput *m_audioOutput;
    QComboBox *m_rateBox = nullptr;
    QMediaPlayer::PlaybackState m_playerState = QMediaPlayer::StoppedState;
    // bool m_playerMuted = false;
};

#endif // AUDIOPLAYERWIDGET_H
