#include "mediasplitter.h"
#include "qfileinfo.h"
#include "qurl.h"
#include <iostream>

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
    // qint64 position = (3600000*timeStamps[0].hour() + 60000*timeStamps[0].minute() + 1000*timeStamps[0].second() + timeStamps[0].msec()) / 1000;
    // for (QTime t : timeStamps) {
    //     std::cerr << "Time Counter " << ++counter << ": " <<3600000*t.hour() + 60000*t.minute() + 1000*t.second() + t.msec() << std::endl << "Position: " << position << std::endl;
    //     position = (3600000*t.hour() + 60000*t.minute() + 1000*t.second() + t.msec()) / 1000;
    // }
    // std::cerr << timeStamps[0].toString().toStdString() << std::endl;

    // std::cerr << "Position: " << position/1000 << std::endl;
    QUrl furl = QUrl::fromLocalFile(mediaFileName);

    QString slash = "\\";
    if(_WIN32)
        slash = "/";
    QString path = QFileInfo(mediaFileName).path();
    QString currentTimeInStr = QTime::currentTime().toString().replace(":", "-");
    QString fileNameWithoutExt = QFileInfo(furl.path()).baseName();
    QString fileNameWithExt = QFileInfo(furl.path()).fileName();
    QString outputDir = "Splitted-Media-Output" +
                                slash + currentTimeInStr + "_" + fileNameWithoutExt;
    QString outputFilePath = outputDir +
                             slash + QString::number(counter) + "_" + fileNameWithExt;
    std::cerr << outputFilePath.toStdString() << std::endl;
    uint64_t startSeconds = 0;
    uint64_t endSeconds = (3600000*timeStamps[0].hour() + 60000*timeStamps[0].minute() + 1000*timeStamps[0].second() + timeStamps[0].msec()) / 1000;


}

void MediaSplitter::splitMedia() {


}

bool MediaSplitter::splitMediaUtil(uint64_t startSeconds = 0, uint64_t endSeconds = 0)
{

    QUrl furl = QUrl::fromLocalFile(mediaFileName);
    QString slash = "\\";
    if(_WIN32)
        slash = "/";
    QString outputFilePath = QFileInfo(mediaFileName).path() +
                             slash + "out-" + QString::number(counter) +
                             QFileInfo(furl.path()).fileName();

    std::cerr << "CP: " << QFileInfo(mediaFileName).path().toStdString();
    std::cerr << "Outfile: " << outputFilePath.toStdString() << std::endl;
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
            throw std::runtime_error(QString("Failed to open the input file '%1'.").arg(mediaFileName).toStdString().c_str());
        }

        operationResult = avformat_find_stream_info(avInputFormatContext, 0);
        if (operationResult < 0) {
            throw std::runtime_error(QString("Failed to retrieve the input stream information.").toStdString().c_str());
        }

        avformat_alloc_output_context2(&avOutputFormatContext, NULL, NULL, outputFilePath.toStdString().c_str());
        if (!avOutputFormatContext) {
            operationResult = AVERROR_UNKNOWN;
            throw std::runtime_error(QString("Failed to create the output context.").toStdString().c_str());
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
                throw std::runtime_error(QString("Failed to allocate the output stream.").toStdString().c_str());
            }

            operationResult = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
            if (operationResult < 0) {
                throw std::runtime_error(
                    QString("Failed to copy codec parameters from input stream to output stream.").toStdString().c_str());
            }
            outStream->codecpar->codec_tag = 0;
        }

        if (!(avOutputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            operationResult = avio_open(&avOutputFormatContext->pb, outputFilePath.toStdString().c_str(), AVIO_FLAG_WRITE);
            if (operationResult < 0) {
                throw std::runtime_error(
                    QString("Failed to open the output file '%1'.").arg(outputFilePath).toStdString().c_str());
            }
        }

        operationResult = avformat_write_header(avOutputFormatContext, NULL);
        if (operationResult < 0) {
            throw std::runtime_error(QString("Error occurred when opening output file.").toStdString().c_str());
        }

        operationResult = avformat_seek_file(avInputFormatContext, -1, INT64_MIN, startSeconds * AV_TIME_BASE,
                                             startSeconds * AV_TIME_BASE, 0);
        if (operationResult < 0) {
            throw std::runtime_error(
                QString("Failed to seek the input file to the targeted start position.").toStdString().c_str());
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
                throw std::runtime_error(QString("Failed to mux the packet.").toStdString().c_str());
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
        qCritical("%s", QString("Error occurred: %1.").arg(av_err2str(operationResult)).toStdString().c_str());
        return false;
    }

    return true;
}
