#include "mediasplitter.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qurl.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
}
#include <stdexcept>

MediaSplitter::MediaSplitter(QWidget *parent)
    : QDialog(parent) {


}

MediaSplitter::MediaSplitter(QWidget *parent,
    QString mediaFileName, QList<QTime> timeStamps)
    : counter(0), mediaFileName(mediaFileName), timeStamps(timeStamps)
{

    setWindowTitle("Splitting");

    QUrl furl = QUrl::fromLocalFile(mediaFileName);

    slash = "/";
        // (_WIN32)? ("/"): ("\\");
    path = QFileInfo(mediaFileName).path();
    currentTimeInStr = QTime::currentTime().toString().replace(":", "-");
    fileNameWithoutExt = QFileInfo(furl.path()).baseName();
    fileNameWithExt = QFileInfo(furl.path()).fileName();

    QString dir(QDir::currentPath()+ slash + "Splitted-Media");
    if (!QDir(dir).exists())
        QDir(dir).mkpath(dir);

    outputDir = dir + slash + currentTimeInStr + "_" + fileNameWithoutExt;

    QDir(dir).mkdir(outputDir);
    outputFilePath = outputDir + slash + QString::number(counter++) + "_" + fileNameWithExt;
    startSeconds = 0;
    endSeconds = (3600000*timeStamps[0].hour() + 60000*timeStamps[0].minute() + 1000*timeStamps[0].second() + timeStamps[0].msec()) / 1000;
    vBoxLayout = new QVBoxLayout(this);


    pBar = new QProgressBar(this);
    pBar->setMaximum(100);
    pBar->setMinimum(0);
    vBoxLayout->addWidget(pBar);

    sBar = new QStatusBar(this);
    sBar->showMessage("");
    vBoxLayout->addWidget(sBar);

    splitMedia();
}

void MediaSplitter::splitMedia() {

    splitMediaUtil(startSeconds, endSeconds);

    for (uint64_t i = 0; i < timeStamps.size()-1; i++) {

        pBar->setValue(i/timeStamps.size()*100);

        startSeconds = endSeconds;
        endSeconds = (3600000*timeStamps[i+1].hour() + 60000*timeStamps[i+1].minute() + 1000*timeStamps[i+1].second() + timeStamps[i+1].msec()) / 1000;
        outputFilePath = outputDir + slash + QString::number(counter++) + "_" + fileNameWithExt;

        splitMediaUtil(startSeconds, endSeconds);
    }
    pBar->setValue(100);
    this->close();

}

bool MediaSplitter::splitMediaUtil(uint64_t startSeconds = 0, uint64_t endSeconds = 0)
{

    int operationResult;

    AVPacket* avPacket = NULL;
    AVFormatContext* avInputFormatContext = NULL;
    AVFormatContext* avOutputFormatContext = NULL;

    avPacket = av_packet_alloc();
    if (!avPacket) {
        qCritical("Failed to allocate AVPacket.");
        return false;
    }

    try {
        operationResult = avformat_open_input(&avInputFormatContext, mediaFileName.toStdString().c_str(), 0, 0);
        if (operationResult < 0) {
            sBar->showMessage(QString("Failed to open the input file '%1'.").arg(mediaFileName).toStdString().c_str());
            qCritical("%s", QString("Failed to open the input file '%1'.").arg(mediaFileName).toStdString().c_str());
        }

        operationResult = avformat_find_stream_info(avInputFormatContext, 0);
        if (operationResult < 0) {
            sBar->showMessage(QString("Failed to retrieve the input stream information.").toStdString().c_str());
            qCritical("%s", QString("Failed to retrieve the input stream information.").toStdString().c_str());
        }

        avformat_alloc_output_context2(&avOutputFormatContext, NULL, NULL, outputFilePath.toStdString().c_str());
        if (!avOutputFormatContext) {
            operationResult = AVERROR_UNKNOWN;
            sBar->showMessage(QString("Failed to create the output context.").toStdString().c_str());
            qCritical("%s", QString("Failed to create the output context.").toStdString().c_str());
        }

        int streamIndex = 0;
        uint nbs = 0;
        nbs = avInputFormatContext->nb_streams;
        // int streamMapping[nbs];
        // int streamRescaledStartSeconds[nbs];
        // int streamRescaledEndSeconds[nbs];
        std::vector<int> streamMapping(nbs, -1);
        std::vector<int> streamRescaledStartSeconds(nbs, 0);
        std::vector<int> streamRescaledEndSeconds(nbs, 0);

        // Copy streams from the input file to the output file.
        for (int i = 0; i < avInputFormatContext->nb_streams; i++) {
            AVStream* outStream;
            AVStream* inStream = avInputFormatContext->streams[i];

            streamRescaledStartSeconds[i] = av_rescale_q(startSeconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);
            streamRescaledEndSeconds[i] = av_rescale_q(endSeconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);

            if (inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                inStream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                streamMapping[i] = -1;
                continue;
            }

            streamMapping[i] = streamIndex++;

            outStream = avformat_new_stream(avOutputFormatContext, NULL);
            if (!outStream) {
                operationResult = AVERROR_UNKNOWN;
                sBar->showMessage(QString("Failed to allocate the output stream.").toStdString().c_str());
                qCritical("%s", QString("Failed to allocate the output stream.").toStdString().c_str());
            }

            operationResult = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
            if (operationResult < 0) {
                sBar->showMessage(QString("Failed to copy codec parameters from input stream to output stream.").toStdString().c_str());
                qCritical(
                    "%s", QString("Failed to copy codec parameters from input stream to output stream.").toStdString().c_str());
            }
            outStream->codecpar->codec_tag = 0;
        }

        if (!(avOutputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            operationResult = avio_open(&avOutputFormatContext->pb, outputFilePath.toStdString().c_str(), AVIO_FLAG_WRITE);
            if (operationResult < 0) {
                sBar->showMessage(QString("Failed to open the output file '%1'.").arg(outputFilePath).toStdString().c_str());
                qCritical(
                    "%s", QString("Failed to open the output file '%1'.").arg(outputFilePath).toStdString().c_str());
            }
        }

        operationResult = avformat_write_header(avOutputFormatContext, NULL);
        if (operationResult < 0) {
            sBar->showMessage(QString("Error occurred when opening output file.").toStdString().c_str());
            qCritical("%s", QString("Error occurred when opening output file.").toStdString().c_str());
        }

        operationResult = avformat_seek_file(avInputFormatContext, -1, INT64_MIN, startSeconds * AV_TIME_BASE,
                                             startSeconds * AV_TIME_BASE, 0);
        if (operationResult < 0) {
            sBar->showMessage(QString("Failed to seek the input file to the targeted start position.").toStdString().c_str());
            qCritical(
                "%s", QString("Failed to seek the input file to the targeted start position.").toStdString().c_str());
        }

        while (true) {
            operationResult = av_read_frame(avInputFormatContext, avPacket);
            if (operationResult < 0) break;

            // Skip packets from unknown streams and packets after the end cut position.
            if (avPacket->stream_index >= avInputFormatContext->nb_streams || streamMapping[avPacket->stream_index] < 0 ||
                avPacket->pts > streamRescaledEndSeconds[avPacket->stream_index]) {
                av_packet_unref(avPacket);
                continue;
            }

            avPacket->stream_index = streamMapping[avPacket->stream_index];

            // Shift the packet to its new position by subtracting the rescaled start seconds.
            avPacket->pts -= streamRescaledStartSeconds[avPacket->stream_index];
            avPacket->dts -= streamRescaledStartSeconds[avPacket->stream_index];

            av_packet_rescale_ts(avPacket, avInputFormatContext->streams[avPacket->stream_index]->time_base,
                                 avOutputFormatContext->streams[avPacket->stream_index]->time_base);
            avPacket->pos = -1;

            operationResult = av_interleaved_write_frame(avOutputFormatContext, avPacket);
            if (operationResult < 0) {
                sBar->showMessage(QString("Failed to mux the packet.").toStdString().c_str());
                qCritical("%s", QString("Failed to mux the packet.").toStdString().c_str());
            }
        }

        av_write_trailer(avOutputFormatContext);
    } catch (std::runtime_error e) {
        qCritical("%s", e.what());
    }

    av_packet_free(&avPacket);

    avformat_close_input(&avInputFormatContext);

    if (avOutputFormatContext && !(avOutputFormatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&avOutputFormatContext->pb);
    avformat_free_context(avOutputFormatContext);

    if (operationResult < 0 && operationResult != AVERROR_EOF) {
        if (_WIN32)
            qCritical("%s", QString("Error occurred: %1.").arg(av_err2str(operationResult)).toStdString().c_str());
        else
            qInfo("FFMPEG Error");
        return false;
    }

    return true;
}
