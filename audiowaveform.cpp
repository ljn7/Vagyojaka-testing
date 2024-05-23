#include "audiowaveform.h"
#include "ui_audiowaveform.h"
#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QComboBox>
#include <QAudio>
#include<iostream>

#include"mediaplayer/mediaplayer.h"

//---------------------------- ---
#define AUDIBLE_RANGE_START 20
#define AUDIBLE_RANGE_END   20000
//----------------------------

AudioWaveForm::AudioWaveForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AudioWaveForm)
{
    ui->setupUi(this);

    waveWidget = new QCustomPlot(this);

    //horizontalScrollBar = new QScrollBar(Qt::Horizontal, this);
    //connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &AudioWaveForm::onScrollBarValueChanged);

    waveWidget->yAxis->setRange(-1.0, 1.0);
    waveWidget->xAxis->setRange(0, 8);
    waveWidget->clearGraphs();
    waveWidget->addGraph();
    waveWidget->yAxis->setVisible(false);
    waveWidget->xAxis->setVisible(true);


    waveWidget->graph()->setPen(QPen(Qt::black));

    waveWidget->setVisible(false);
    waveWidget->graph()->setVisible(true);

    waveWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    waveWidget->setMinimumSize(0, 100);

    QBoxLayout *layout2 = new QVBoxLayout;

    layout2->addWidget(waveWidget);
    //layout2->addWidget(horizontalScrollBar);

    setLayout(layout2);

    waveWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |/* QCP::iSelectPlottables |*/ QCP::iSelectItems | QCP::iRangeZoom);

    connect(waveWidget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));
    connect(waveWidget, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));

    waveWidget->setVisible(false);

}

AudioWaveForm::~AudioWaveForm()
{
    delete ui;
    fftw_free(mFftIn);
    fftw_free(mFftOut);
    fftw_destroy_plan(mFftPlan);
}

void AudioWaveForm::getSampleRate(qint64 sampleRate, QBuffer& audioBuffer, qint64 totalDuration)
{
    this->sample_rate = sampleRate;
    this->total_dur = totalDuration;
    std::cerr << "total_duration: " << total_dur << std::endl;
    this->num_sam = sample_rate * (total_dur/1000);
    // factor = num_sam / NUM_SAMPLES;
    qInfo()<<"sample_rate is: :"<<this->sample_rate<<"\n";
    std::cerr<<"sample_rate is: "<<this->sample_rate;
    //qInfo()<<"total duration is: "<<total_dur<<"\n";
    //qInfo()<<"num samples is: "<<num_sam<<"\n";

    mInputBuffer.open(QIODevice::ReadWrite);
    audioBuffer.open(QIODevice::ReadOnly);

    while (!audioBuffer.atEnd()) {
        QByteArray chunk = audioBuffer.read(1024); // Read 1024 bytes at a time
        mInputBuffer.write(chunk);
    }
    mInputBuffer.close();
    audioBuffer.close();

    processBuffer(audioBuffer);
}
void AudioWaveForm::processBuffer(QBuffer& audioBuffer)
{
    /*while(num_sam == 0 && sample_rate == 0){

    }*/
    for (qint64 i = 0; i < num_sam; i ++) {
        mIndices.append((double)i);
        mSamples.append(0);
    }

    double freqStep = (double)sample_rate / (double)(num_sam);
    double f = AUDIBLE_RANGE_START;
    while (f < AUDIBLE_RANGE_END) {
        mFftIndices.append(f);
        f += freqStep;
    }

    // Set up FFT plan *
    mFftIn  = fftw_alloc_real(num_sam);
    mFftOut = fftw_alloc_real(num_sam);
    mFftPlan = fftw_plan_r2r_1d(num_sam, mFftIn, mFftOut, FFTW_R2HC,FFTW_ESTIMATE);

    waveWidget->setVisible(true);
    //qInfo()<<"processing buffer\n";
    mInputBuffer.open(QIODevice::ReadWrite);
    audioBuffer.open(QIODevice::ReadOnly);

    while (!audioBuffer.atEnd()) {
        QByteArray chunk = audioBuffer.read(1024); // Read 1024 bytes at a time
        mInputBuffer.write(chunk);
    }
    mInputBuffer.close();
    audioBuffer.close();
    //processAudioIn();

    processAudioIn();
}

void AudioWaveForm::processAudioIn()
{
    //qInfo()<<"processing audio\n";
    mInputBuffer.open(QIODevice::ReadOnly);
    mInputBuffer.seek(0);
    QByteArray ba = mInputBuffer.readAll();
    int16_t s_max = 0;
    int num_sample = ba.length() / 2;
    int b_pos = 0;
    for (qint64 i = 0; i < num_sample; i ++) {
        int16_t s;
        s = ba.at(b_pos++);
        s |= ba.at(b_pos++) << 8;
        if(s > s_max){
            s_max = s;
        }
    }
    b_pos = 0;
    for (qint64 i = 0; i < num_sample; i ++) {
        int16_t s;
        s = ba.at(b_pos++);
        s |= ba.at(b_pos++) << 8;
        if (s != 0) {
            mSamples.append((double)s / /*32768.0*/(double)s_max);
        } else {
            mSamples.append(0);
        }
    }
    mInputBuffer.buffer().clear();
    mInputBuffer.seek(0);
    mInputBuffer.close();
    //samplesUpdated();
}

void AudioWaveForm::getTimeArray(QVector<QTime> timeArray)
{
    //qInfo()<<"getting time array\n";
    endTime = timeArray;
    int totalSeconds;

    for(int i = 0; i < endTime.size(); ++i)
    {
        totalSeconds = endTime[i].second() + endTime[i].minute() * 60 + endTime[i].hour() * 3600;
        blocktime.append(totalSeconds);
        totalSeconds = 0;
    }
    num_of_blocks = endTime.size();

    plotLines(1);


}
void AudioWaveForm::plotLines(int n)
{
    if(!startLine.isEmpty())
    {
        for(int i = 0; i < startLine.size(); ++i)
        {
            waveWidget->removeItem(startLine[i]);
        }
        startLine.clear();
        waveWidget->replot();
    }
    if(!endLine.isEmpty())
    {
        for(int i = 0; i < endLine.size(); ++i)
        {
            waveWidget->removeItem(endLine[i]);
        }
        endLine.clear();
        waveWidget->replot();
    }

    //qInfo()<<"plotting lines\n";
    int s = 0;
    startLine.clear();
    endLine.clear();
    for(int i = 0; i < num_of_blocks; ++i)
    {
        QCPItemLine* newbeginline = new QCPItemLine(waveWidget);
        QCPItemLine* newendline = new QCPItemLine(waveWidget);
        startLine.append(newbeginline);
        endLine.append(newendline);

        startLine[i]->start->setCoords(s, -1);
        if(i == 0){
            startLine[0]->setSelectable(false);
        }
        startLine[i]->end->setCoords(s, 1);
        startLine[i]->setPen(QPen(Qt::green));
        endLine[i]->start->setCoords(blocktime[i], -1);
        endLine[i]->end->setCoords(blocktime[i], 1);
        endLine[i]->setPen(QPen(Qt::red));

        s = blocktime[i]+1;
    }
    //if(itemsAvailable != 1){
    startCoords.clear();
    endCoords.clear();
    //}

    for(int i = 0; i < num_of_blocks; ++i)
    {
        double startcoordinate = startLine[i]->start->coords().x();
        double endcoordinate = endLine[i]->start->coords().x();
        startCoords.append(startcoordinate);
        endCoords.append(endcoordinate);
    }
    waveWidget->replot();
    if(n == 1)
        setUtteranceNumber(1);
    else
        setUtteranceNumber(2);
    //samplesUpdated();

}

void AudioWaveForm::setUtteranceNumber(int n)
{
    if(!utteranceNumbers.isEmpty())
    {
        for(int i = 0; i < utteranceNumbers.size(); ++i)
        {
            waveWidget->removeItem(utteranceNumbers[i]);
        }
        utteranceNumbers.clear();
        waveWidget->replot();
    }

    //qInfo()<<"setting utterance number\n";
    int s = 0;
    for(int i = 0; i < num_of_blocks; ++i)
    {
        QCPItemText* utteranceNumberText = new QCPItemText(waveWidget);
        utteranceNumberText->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
        utteranceNumberText->position->setType(QCPItemPosition::ptPlotCoords);
        utteranceNumberText->position->setCoords(s + (blocktime[i] - s) / 2.0, 1.1); // Adjust the vertical position

        utteranceNumberText->setText(QString::number(i + 1)); // Display utterance number
        utteranceNumberText->setFont(QFont(font().family(), 8)); // Adjust font size if needed

        // Add text item to the vector
        utteranceNumbers.append(utteranceNumberText);

        s = blocktime[i]+1;
    }
    waveWidget->replot();
    linesAvailable = 1;

    if(n == 1)
        samplesUpdated();
}

void AudioWaveForm::updateUtterances(int index)
{

    utteranceNumbers[index]->position->setCoords((startCoords[index] + endCoords[index])/2.0, 1.1);
    waveWidget->replot();
}

void AudioWaveForm::getDuration(qint64 total_time)
{
    total_dur = total_time;
    //qInfo()<<"Duration is: "<<total_dur<<"\n";
    //sample_rate = num_sam/(total_dur/1000);
}
void AudioWaveForm::samplesUpdated()
{

    //qInfo()<<"Updating samples\n";
    int n = mSamples.length();
    if (n > /*96000*/ num_sam) mSamples = mSamples.mid(n - (num_sam), -1);
    memcpy(mFftIn, mSamples.data(), num_sam * sizeof(double));
    fftw_execute(mFftPlan);

    QVector<double> fftVec;
    double totalDuration = total_dur/1000.0;
    // Create a vector for time values from 0 to totalDuration
    QVector<double> timeValues;
    double timeStep = totalDuration/((num_sam) - 1);
    for (qint64 i = 0; i < num_sam; i++) {
        timeValues.append(i * timeStep);
    }
    /*
    for (qint64 i = 0; i < NUM_SAMPLES; i++) {
        fftVec.append(abs(mFftOut[i]));
    }*/

    waveWidget->graph(0)->setData(timeValues, mSamples);
    waveWidget->xAxis->rescale();
    waveWidget->replot();
    waveWidget->setVisible(true);
}

// Slot to handle line movement

void AudioWaveForm::onMousePress(QMouseEvent *event) {
    // Check if a line is clicked and set it as selected
    if(linesAvailable == 1){
        flag1*=(-1);
        if(flag1 == 1){
            for(int i = 0; i < num_of_blocks; ++i)
            {
                if(startLine[i]->selectTest(event->pos(), false) >= 0)
                {
                    //startLine[i]->setSelectable(true);
                    //qInfo()<<"start line selected/deselected\n";
                    startLine[i]->setSelected(!startLine[i]->selected());
                    deselectLines(startLine, i, num_of_blocks);
                    deselectLines(endLine, -1, num_of_blocks);
                    //flag1*=(-1);
                    break;
                }
                else if(endLine[i]->selectTest(event->pos(), false) >= 0)
                {
                    //endLine[i]->setSelectable(true);
                    endLine[i]->setSelected(!endLine[i]->selected());
                    deselectLines(startLine, -1, num_of_blocks);
                    deselectLines(endLine, i, num_of_blocks);
                    //flag1*=(-1);
                    break;
                }
                else
                {
                    deselectLines(startLine, -1, num_of_blocks);
                    deselectLines(endLine, -1, num_of_blocks);
                    //flag1*=(-1);
                }
            }
        }
        else
        {
            waveWidget->deselectAll();
            flag1*=(-1);
        }
    }
    //qInfo()<<flag1<<"\n";

}

void AudioWaveForm::deselectLines(QVector<QCPItemLine *> & lines, int index, int num_of_lines)
{
    for(int i = 0; i < num_of_lines; ++i)
    {
        if(i != index){
            lines[i]->setSelected(false);
        }
    }
}

void AudioWaveForm::onMouseMove(QMouseEvent *event) {
    if(linesAvailable == 1){
        for(int i = 0; i < num_of_blocks; ++i) {
            if (startLine[i]->selected() || endLine[i]->selected()) {
                double x = waveWidget->xAxis->pixelToCoord(event->pos().x());

                if(startLine[i]->selected()){
                    // Check if there's a next endLine and a previous endLine
                    if (i + 1 < num_of_blocks && i - 1 >= 0) {
                        double nextEndX = endLine[i]->start->coords().x();
                        double prevEndX = endLine[i - 1]->start->coords().x();

                        // Limit the movement of startLine[i] between nextEndX and prevEndX
                        if (x >= prevEndX && x <= nextEndX) {
                            if (startLine[i]->selected()) {
                                startLine[i]->start->setCoords(x, -1);
                                startLine[i]->end->setCoords(x, 1);
                                startCoords[i] = startLine[i]->start->coords().x();
                                updateUtterances(i);
                                QTime a(0,0,0);
                                a = a.addSecs(int(x));
                                updateTime(i, a);
                            }
                        }
                    }

                    // Check if there's only a next endLine
                    else if (i + 1 < num_of_blocks) {
                        double nextEndX = endLine[i]->start->coords().x();

                        // Limit the movement of startLine[i] before nextEndX
                        if (x <= nextEndX) {
                            if (startLine[i]->selected()) {
                                startLine[i]->start->setCoords(x, -1);
                                startLine[i]->end->setCoords(x, 1);
                                startCoords[i] = startLine[i]->start->coords().x();
                                updateUtterances(i);
                            }
                        }
                    }

                    // Check if there's only a previous endLine
                    else if (i - 1 >= 0) {
                        double prevEndX = endLine[i - 1]->start->coords().x();

                        // Limit the movement of startLine[i] after prevEndX
                        if (x >= prevEndX) {
                            if (startLine[i]->selected()) {
                                startLine[i]->start->setCoords(x, -1);
                                startLine[i]->end->setCoords(x, 1);
                                startCoords[i] = startLine[i]->start->coords().x();
                                updateUtterances(i);
                            }
                        }
                    }
                }
                else if(endLine[i]->selected())
                {
                    if (i + 1 < num_of_blocks && i - 1 >= 0) {
                        double nextEndX = startLine[i+1]->start->coords().x();
                        double prevEndX = startLine[i]->start->coords().x();

                        // Limit the movement of startLine[i] between nextEndX and prevEndX
                        if (x >= prevEndX && x <= nextEndX) {
                            if (endLine[i]->selected()) {
                                endLine[i]->start->setCoords(x, -1);
                                endLine[i]->end->setCoords(x, 1);
                                endCoords[i] = endLine[i]->start->coords().x();
                                updateUtterances(i);

                                QTime a(0,0,0);
                                a = a.addSecs(int(x));
                                updateTime(i, a);
                            }
                        }
                    }

                    // Check if there's only a next endLine
                    else if (i + 1 < num_of_blocks) {
                        double nextEndX = startLine[i+1]->start->coords().x();

                        // Limit the movement of startLine[i] before nextEndX
                        if (x <= nextEndX) {
                            if (endLine[i]->selected()) {
                                endLine[i]->start->setCoords(x, -1);
                                endLine[i]->end->setCoords(x, 1);
                                endCoords[i] = endLine[i]->start->coords().x();
                                updateUtterances(i);
                            }
                        }
                    }

                    // Check if there's only a previous endLine
                    else if (i - 1 >= 0) {
                        double prevEndX = startLine[i]->start->coords().x();

                        // Limit the movement of startLine[i] after prevEndX
                        if (x >= prevEndX) {
                            if (endLine[i]->selected()) {
                                endLine[i]->start->setCoords(x, -1);
                                endLine[i]->end->setCoords(x, 1);
                                endCoords[i] = endLine[i]->start->coords().x();
                                updateUtterances(i);

                                QTime a(0,0,0);
                                a = a.addSecs(int(x));
                                updateTime(i, a);
                            }
                        }
                    }
                }

                waveWidget->replot();
            }
        }
    }
}

