#ifndef AUDIOPLAYERWIDGET_H
#define AUDIOPLAYERWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>

class AudioPlayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit AudioPlayerWidget(const QString &audioFilePath, QWidget *parent = nullptr)
        : QWidget(parent), m_audioFilePath(audioFilePath) {
        // Create the media player
        m_audioOutput = new QAudioOutput(this);
        m_mediaPlayer = new QMediaPlayer(this);
        m_mediaPlayer->setSource(QUrl::fromLocalFile(audioFilePath));
        m_mediaPlayer->setAudioOutput(m_audioOutput);

        // Create the play/pause button
        m_playButton = new QPushButton("Play", this);
        connect(m_playButton, &QPushButton::clicked, this, &AudioPlayerWidget::togglePlayback);

        // Create the position slider
        m_slider = new QSlider(Qt::Horizontal, this);
        connect(m_slider, &QSlider::sliderMoved, this, &AudioPlayerWidget::seek);

        // Create the layout
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addWidget(m_playButton);
        hLayout->addWidget(m_slider);

        m_labelDuration = new QLabel();
        hLayout->addWidget(m_labelDuration);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addLayout(hLayout);
        setLayout(layout);

        // Connect signals
        connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &AudioPlayerWidget::positionChanged);
        connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &AudioPlayerWidget::durationChanged);
    }

    void togglePlayback() {
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            m_mediaPlayer->pause();
            m_playButton->setText("Play");
        } else {
            m_mediaPlayer->play();
            m_playButton->setText("Pause");
        }
    }

    void setPosition(int position) {
        m_mediaPlayer->setPosition(position);
    }

    void positionChanged(qint64 progress) {
        if (!m_slider->isSliderDown())
            m_slider->setValue(progress);

        updateDurationInfo(progress / 1000);
    }

    void durationChanged(qint64 duration) {
        m_slider->setRange(0, duration);
        m_duration = duration / 1000;
        m_slider->setMaximum(duration);
    }

    void updateDurationInfo(qint64 currentInfo) {
        QString tStr;
        if (currentInfo || m_duration) {
            QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60, currentInfo % 60,
                              (currentInfo * 1000) % 1000);
            QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60, m_duration % 60,
                            (m_duration * 1000) % 1000);
            QString format = "mm:ss";
            if (m_duration > 3600)
                format = "hh:mm:ss";
            tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
        }
        m_labelDuration->setText(tStr);
    }


    void seek(int mseconds)
    {
        m_mediaPlayer->setPosition(mseconds);
    }

private:
    QString m_audioFilePath;
    QMediaPlayer *m_mediaPlayer;
    QPushButton *m_playButton;
    QSlider *m_slider;
    qint64 m_duration = 0;
    QLabel *m_labelDuration;
    QAudioOutput *m_audioOutput;
};

#endif // AUDIOPLAYERWIDGET_H
