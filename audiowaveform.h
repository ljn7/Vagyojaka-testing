#ifndef AUDIOWAVEFORM_H
#define AUDIOWAVEFORM_H

#include <QWidget>

#include <QMediaPlayer>
#include <QAbstractButton>
#include <QAbstractSlider>
#include <QComboBox>
#include <QLabel>
#include<QDebug>
#include<QMediaMetaData>

//---------------------------------------
#include"mediaplayer/qcustomplot.h"
#include<QVector>
#include"mediaplayer/fftw3.h"
#include<QAudioFormat>

namespace Ui {
class AudioWaveForm;
}

class AudioWaveForm : public QWidget
{
    Q_OBJECT

public:
    explicit AudioWaveForm(QWidget *parent = nullptr);
    ~AudioWaveForm();

public slots:
    void getDuration(qint64 total_time);
    void onMousePress(QMouseEvent *event);
    void getTimeArray(QVector<QTime> timeArray);
    void getSampleRate(qint64 sampleRate, QBuffer &audioBuffer, qint64 totalDuration);
    void processBuffer(QBuffer &audioBuffer);

private slots:
    void processAudioIn();

    void onMouseMove(QMouseEvent *event);

signals:
    void updateTime(int block_num, QTime endTime);
private:
    Ui::AudioWaveForm *ui;
    //-------------------------------------------
    QCustomPlot *waveWidget;
    void samplesUpdated();
    void plotLines(int n);
    void deselectLines(QVector<QCPItemLine*> &lines, int index, int num_of_lines);
    void setUtteranceNumber(int n);
    void updateUtterances(int ind);

    QBuffer mInputBuffer;
    qint64 tot_duration;

    //qint64 mDuration;
    QVector<double> mFftIndices;

    fftw_plan mFftPlan;
    double *mFftIn;
    double *mFftOut;

    QVector<double> mSamples;
    QVector<double> mIndices;

    qint64 total_dur;
    int flag1 = 1;
    int linesAvailable = -1;
    int num_of_blocks;
    qint64 sample_rate = 0;
    qint64 num_sam = 0;
    int factor = 1;
    QVector<int> blocktime;
    QVector<QTime> endTime;

    QVector<QCPItemLine*> startLine;
    QVector<QCPItemLine*> endLine;
    QVector<QCPItemText*> utteranceNumbers;
    QVector<double> startCoords;
    QVector<double> endCoords;
};

#endif // AUDIOWAVEFORM_H
