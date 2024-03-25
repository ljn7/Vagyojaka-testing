#include "mediaplayer.h"
#include "qapplication.h"
#include <iostream>

MediaPlayer::MediaPlayer(QWidget *parent)
    : QMediaPlayer(parent)
{
    //Qt6
    this->supportedFormats = {
        "Video Files (*.avi *.divx *.amv *.mpg *.mpeg *.mpe *.m1v *.m2v *.mpv2 "
        "*.mp2v *.m2p *.vob *.evo *.mod *.ts *.m2ts *.m2t *.mts *.pva *.tp *.tpr "
        "*.mp4 *.m4v *.mp4v *.mpv4 *.hdmov *.mov *.3gp *.3gpp *.3g2 *.3gp2 *.mkv "
        "*.webm *.ogm *.ogv *.flv *.f4v *.wmv *.asf *.rmvb *.rm *.dv *.mxf *.dav)",

        "Audio Files (*.mp3 *.ogg *.oga *.mka *.wav *.m4a *.aac *.flac *.wv *.mpc *.ape *.alac *.amr "
        "*.tta *.ac3 *.dts *.ra *.opus *.spx *.aif *.aiff *.aifc *.caf *.tak *.shr)",

        "All Files (*)"
    };
    QString iniPath = QApplication::applicationDirPath() + "/" + "config.ini";
    settings = new QSettings(iniPath, QSettings::IniFormat);
}

QTime MediaPlayer::elapsedTime()
{
    return getTimeFromPosition(position());
}

QTime MediaPlayer::durationTime()
{
    return getTimeFromPosition(duration());
}

void MediaPlayer::setPositionToTime(const QTime& time)
{
    if (time.isNull())
        return;
    qint64 position = 3600000*time.hour() + 60000*time.minute() + 1000*time.second() + time.msec();
    std::cerr << "Media Player: " << 3600000*time.hour() + 60000*time.minute() + 1000*time.second() + time.msec() << std::endl
              << "Position in MediaPlayer: " << position << std::endl;
    setPosition(position);
}

QString MediaPlayer::getMediaFileName()
{
    return std::as_const(m_mediaFileName);
}

QString MediaPlayer::getPositionInfo()
{
    QString format = "mm:ss";
    if (durationTime().hour() != 0)
        format = "hh:mm:ss";

    return elapsedTime().toString(format) + " / " + durationTime().toString(format);
}

void MediaPlayer::loadMediaFromUrl(QUrl *fileUrl)
{
    QFile MediaFile(fileUrl->toLocalFile());
    QFileInfo filedir(MediaFile);
    QString dirInString=filedir.dir().path();
    settings->setValue("mediaDir",dirInString);
    QString slash = "/";
        // (_WIN32)? ("/"): ("\\");
    m_mediaFileName = dirInString + slash + fileUrl->fileName();
    //Qt6
    // setMedia(*fileUrl);
    setSource(*fileUrl);
    emit message("Opened file " + fileUrl->fileName());
    emit openMessage(fileUrl->fileName());
    play();
}

void MediaPlayer::open()
{
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Media"));
    //Qt6 porting
    // QStringList supportedMimeTypes = QMediaPlayer::supportedMimeTypes();
    // if (!supportedMimeTypes.isEmpty())
    //     fileDialog.setMimeTypeFilters(supportedMimeTypes);

    // fileDialog.setMimeTypeFilters(supportedFormats);


    fileDialog.setNameFilters(supportedFormats);
    if(settings->value("mediaDir").toString()=="")
        fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    else
        fileDialog.setDirectory(settings->value("mediaDir").toString());
    if (fileDialog.exec() == QDialog::Accepted) {
        QUrl *fileUrl = new QUrl(fileDialog.selectedUrls().constFirst());
        loadMediaFromUrl(fileUrl);
    }
}

void MediaPlayer::seek(int seconds)
{
    if (elapsedTime().addSecs(seconds) > durationTime())
        setPosition(duration());
    else if (elapsedTime().addSecs(seconds).isNull())
        setPosition(0);
    else
        setPositionToTime(elapsedTime().addSecs(seconds));
}

QTime MediaPlayer::getTimeFromPosition(const qint64& position)
{
    auto milliseconds = position % 1000;
    auto seconds = (position/1000) % 60;
    auto minutes = (position/60000) % 60;
    auto hours = (position/3600000) % 24;

    return QTime(hours, minutes, seconds, milliseconds);
}

//Qt6
void MediaPlayer::togglePlayback()
{
    if (playbackState() == MediaPlayer::PausedState || playbackState() == MediaPlayer::StoppedState)
        play();
    else if (playbackState() == MediaPlayer::PlayingState)
        pause();
}


// Qt6
// void MediaPlayer::setVolume(int volume)
// {
//     audioOutput()->setVolume(volume);
// }

// // Qt6

// void MediaPlayer::setMuted(bool muted)
// {
//     audioOutput()->setMuted(muted);
// }


