#include "audioplayerwidget.h"
#include "qboxlayout.h"
#include "qtoolbutton.h"


AudioPlayerWidget::AudioPlayerWidget(const QString &audioFilePath, QWidget *parent)
    : QWidget(parent), m_audioFilePath(audioFilePath) {
    // Create the media player

    m_audioOutput = new QAudioOutput(this);

    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(audioFilePath));
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    // Create the play/pause button
    m_playButton = new QToolButton(this);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QAbstractButton::clicked, this, &AudioPlayerWidget::togglePlayback);
    connect(this, &AudioPlayerWidget::play, m_mediaPlayer, &QMediaPlayer::play);
    connect(this, &AudioPlayerWidget::pause, m_mediaPlayer, &QMediaPlayer::pause);

    m_stopButton = new QToolButton(this);
    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopButton->setEnabled(false);

    m_playButton->setIconSize(QSize(10, 10));
    m_stopButton->setIconSize(QSize(10, 10));

    connect(m_stopButton, &QAbstractButton::clicked, m_mediaPlayer, &QMediaPlayer::stop);

    // m_muteButton = new QToolButton(this);
    // m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    // connect(m_muteButton, &QAbstractButton::clicked, this, &AudioPlayerWidget::muteClicked);


    // Create the position slider
    m_slider = new QSlider(Qt::Horizontal, this);
    connect(m_slider, &QSlider::sliderMoved, this, &AudioPlayerWidget::seek);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_rateBox = new QComboBox(this);
    m_rateBox->addItem("0.5x", QVariant(0.5));
    m_rateBox->addItem("0.7x", QVariant(0.7));
    m_rateBox->addItem("1.0x", QVariant(1.0));
    m_rateBox->addItem("1.5x", QVariant(1.5));
    m_rateBox->addItem("2.0x", QVariant(2.0));
    m_rateBox->addItem("2.5x", QVariant(2.5));
    m_rateBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // Set size policy
    m_rateBox->setFont(QFont(m_rateBox->font().family(), 8));
    m_rateBox->setCurrentIndex(2);
    connect(m_rateBox, QOverload<int>::of(&QComboBox::activated), this, &AudioPlayerWidget::updateRate);

    // Create the layout
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_playButton);
    hLayout->addWidget(m_stopButton);
    // hLayout->addWidget(m_muteButton);
    hLayout->addWidget(m_slider);
    hLayout->addWidget(m_rateBox);

    m_labelDuration = new QLabel();
    hLayout->addWidget(m_labelDuration);
    // hLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(hLayout);
    layout->setContentsMargins(0, 0, 0, 0);
    // layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);

    // Connect signals
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &AudioPlayerWidget::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &AudioPlayerWidget::durationChanged);
    connect(this, &AudioPlayerWidget::changeRate, m_mediaPlayer, &QMediaPlayer::setPlaybackRate);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &AudioPlayerWidget::setState);
    // connect(m_audioOutput, &QAudioOutput::mutedChanged, this, &AudioPlayerWidget::setMuted);
}

void AudioPlayerWidget::togglePlayback() {
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause();
        m_playButton->setText("Play");
    } else {
        m_mediaPlayer->play();
        m_playButton->setText("Pause");
    }
}

void AudioPlayerWidget::setPosition(int position) {
    m_mediaPlayer->setPosition(position);
}

void AudioPlayerWidget::positionChanged(qint64 progress) {
    if (!m_slider->isSliderDown())
        m_slider->setValue(progress);

    updateDurationInfo(progress / 1000);
}

void AudioPlayerWidget::durationChanged(qint64 duration) {
    m_slider->setRange(0, duration);
    m_duration = duration / 1000;
    m_slider->setMaximum(duration);
}

// bool AudioPlayerWidget::isMuted() const
// {
//     return m_playerMuted;
// }

void AudioPlayerWidget::updateDurationInfo(qint64 currentInfo) {
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


qreal AudioPlayerWidget::playbackRate() const {
    return m_rateBox->itemData(m_rateBox->currentIndex()).toDouble();
}


void AudioPlayerWidget::updateRate()
{
    m_mediaPlayer->setPlaybackRate(playbackRate());
}

void AudioPlayerWidget::seek(int mseconds)
{
    m_mediaPlayer->setPosition(mseconds);
}


void AudioPlayerWidget::setState(QMediaPlayer::PlaybackState state)
{
    if (state != m_playerState) {
        m_playerState = state;

        switch (state) {
        case QMediaPlayer::StoppedState:
            m_stopButton->setEnabled(false);
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        case QMediaPlayer::PlayingState:
            m_stopButton->setEnabled(true);
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
            m_stopButton->setEnabled(true);
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        }
    }
}

void AudioPlayerWidget::playClicked()
{
    switch (m_playerState) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        m_mediaPlayer->play();
        break;
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    }
}

QMediaPlayer::PlaybackState AudioPlayerWidget::state() const
{
    return m_playerState;
}

// void AudioPlayerWidget::muteClicked()
// {
//     m_mediaPlayer->audioOutput()->setMuted(!m_playerMuted);
// }

// void AudioPlayerWidget::setMuted(bool muted)
// {
//     if (muted != m_playerMuted) {
//         m_playerMuted = muted;

//         m_muteButton->setIcon(style()->standardIcon(muted
//                                                         ? QStyle::SP_MediaVolumeMuted
//                                                         : QStyle::SP_MediaVolume));
//     }
// }

AudioPlayerWidget::~AudioPlayerWidget() {

}
