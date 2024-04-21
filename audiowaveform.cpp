#include "audiowaveform.h"
#include "ui_audiowaveform.h"
#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QComboBox>
#include <QAudio>

#include"mediaplayer/mediaplayer.h"

//----------------------------
#define AUDIBLE_RANGE_START 20
#define AUDIBLE_RANGE_END   20000
#define NUM_SAMPLES 7200000  //96000
#define SAMPLE_FREQ 16000  //48000
//----------------------------

AudioWaveForm::AudioWaveForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AudioWaveForm)
{
    ui->setupUi(this);

    waveWidget = new QCustomPlot(this);

    waveWidget->yAxis->setRange(-1.0, 1.0);
    waveWidget->xAxis->setRange(0, 8);
    waveWidget->clearGraphs();
    waveWidget->addGraph();
    waveWidget->yAxis->setVisible(false);
    waveWidget->xAxis->setVisible(true);


    waveWidget->graph()->setPen(QPen(Qt::black));

    waveWidget->setVisible(true);
    waveWidget->graph()->setVisible(true);

    waveWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    waveWidget->setMinimumSize(0, 100);

    QBoxLayout *layout2 = new QVBoxLayout;

    layout2->addWidget(waveWidget);

    setLayout(layout2);

    waveWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |/* QCP::iSelectPlottables |*/ QCP::iSelectItems | QCP::iRangeZoom);

    //connect(waveWidget->xAxis, QOverload<const QCPAxis&>::of(&QCPAxis::rangeChanged), this, &AudioWaveForm::onXAxisRangeChanged);
    connect(waveWidget->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(onXAxisRangeChanged(const QCPRange&)));


    for (int i = 0; i < NUM_SAMPLES; i ++) {
        mIndices.append((double)i);
        mSamples.append(0);
    }

    double freqStep = (double)SAMPLE_FREQ / (double)NUM_SAMPLES;
    double f = AUDIBLE_RANGE_START;
    while (f < AUDIBLE_RANGE_END) {
        mFftIndices.append(f);
        f += freqStep;
    }

    /* Set up FFT plan */
    mFftIn  = fftw_alloc_real(NUM_SAMPLES);
    mFftOut = fftw_alloc_real(NUM_SAMPLES);
    mFftPlan = fftw_plan_r2r_1d(NUM_SAMPLES, mFftIn, mFftOut, FFTW_R2HC,FFTW_ESTIMATE);
    //-------------------------------------------

    connect(waveWidget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));
    connect(waveWidget, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
}

AudioWaveForm::~AudioWaveForm()
{
    delete ui;
    fftw_free(mFftIn);
    fftw_free(mFftOut);
    fftw_destroy_plan(mFftPlan);
}

void AudioWaveForm::processBuffer(QBuffer& audioBuffer)
{
    //waveWidget->clearGraphs();
    mInputBuffer.open(QIODevice::ReadWrite);
    audioBuffer.open(QIODevice::ReadOnly);

    while (!audioBuffer.atEnd()) {
        QByteArray chunk = audioBuffer.read(1024); // Read 1024 bytes at a time
        mInputBuffer.write(chunk);
    }
    mInputBuffer.close();
    audioBuffer.close();
    processAudioIn();
}

void AudioWaveForm::processAudioIn()
{
    mInputBuffer.open(QIODevice::ReadOnly);
    mInputBuffer.seek(0);
    QByteArray ba = mInputBuffer.readAll();

    int num_samples = ba.length() / 2;
    int b_pos = 0;
    for (int i = 0; i < num_samples; i ++) {
        int16_t s;
        s = ba.at(b_pos++);
        s |= ba.at(b_pos++) << 8;
        if (s != 0) {
            mSamples.append((double)s / 32768.0);
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
    endTime = timeArray;
    int totalSeconds;

    for(int i = 0; i < endTime.size(); ++i)
    {
        totalSeconds = endTime[i].second() + endTime[i].minute() * 60 + endTime[i].hour() * 3600;
        blocktime.append(totalSeconds);
        totalSeconds = 0;
    }
    num_of_blocks = endTime.size();

    plotLines();


}
void AudioWaveForm::plotLines()
{
    int s = 0;
    for(int i = 0; i < num_of_blocks; ++i)
    {
        QCPItemLine* newbeginline = new QCPItemLine(waveWidget);
        QCPItemLine* newendline = new QCPItemLine(waveWidget);
        startLine.append(newbeginline);
        endLine.append(newendline);

        startLine[i]->start->setCoords(s, -1);
        startLine[i]->end->setCoords(s, 1);
        startLine[i]->setPen(QPen(Qt::green));
        endLine[i]->start->setCoords(blocktime[i], -1);
        endLine[i]->end->setCoords(blocktime[i], 1);
        endLine[i]->setPen(QPen(Qt::red));

        s = blocktime[i]+1;
    }

    for(int i = 0; i < num_of_blocks; ++i)
    {
        double startcoordinate = startLine[i]->start->coords().x();
        double endcoordinate = endLine[i]->start->coords().x();
        startCoords.append(startcoordinate);
        endCoords.append(endcoordinate);
    }
    waveWidget->replot();
    setUtteranceNumber();

    //samplesUpdated();

}

void AudioWaveForm::setUtteranceNumber()
{
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

    samplesUpdated();
}

void AudioWaveForm::updateUtterances(int index)
{

    utteranceNumbers[index]->position->setCoords((startCoords[index] + endCoords[index])/2.0, 1.1);
    waveWidget->replot();
}

void AudioWaveForm::getDuration(double total_time)
{
    total_dur = total_time;
}
void AudioWaveForm::samplesUpdated()
{

    int n = mSamples.length();
    if (n > /*96000*/ NUM_SAMPLES) mSamples = mSamples.mid(n - NUM_SAMPLES, -1);
    memcpy(mFftIn, mSamples.data(), NUM_SAMPLES * sizeof(double));
    fftw_execute(mFftPlan);

    QVector<double> fftVec;
    double totalDuration = total_dur/1000.0;
    // Create a vector for time values from 0 to totalDuration
    QVector<double> timeValues;
    double timeStep = totalDuration/(NUM_SAMPLES - 1);
    for (int i = 0; i < NUM_SAMPLES; i++) {
        timeValues.append(i * timeStep);
    }

    /*for (int i = (NUM_SAMPLES / SAMPLE_FREQ) * AUDIBLE_RANGE_START;
         i < (NUM_SAMPLES / SAMPLE_FREQ) * AUDIBLE_RANGE_END;
         i++) {
        fftVec.append(abs(mFftOut[i]));
    }*/

    for (int i = 0; i < NUM_SAMPLES; i++) {
        fftVec.append(abs(mFftOut[i]));
    }

    waveWidget->graph(0)->setData(timeValues, mSamples);
    waveWidget->xAxis->rescale();
    waveWidget->replot();
}

// Slot to handle line movement
void AudioWaveForm::onMousePress(QMouseEvent *event) {
    // Check if a line is clicked and set it as selected
    flag1*=(-1);
    if(flag1 == 1){
        for(int i = 0; i < num_of_blocks; ++i)
        {
            if(startLine[i]->selectTest(event->pos(), false) >= 0)
            {
                //startLine[i]->setSelectable(true);
                startLine[i]->setSelected(!startLine[i]->selected());
                deselectLines(startLine, i, num_of_blocks);
                deselectLines(endLine, -1, num_of_blocks);
                break;
            }
            else if(endLine[i]->selectTest(event->pos(), false) >= 0)
            {
                //endLine[i]->setSelectable(true);
                endLine[i]->setSelected(!endLine[i]->selected());
                deselectLines(startLine, -1, num_of_blocks);
                deselectLines(endLine, i, num_of_blocks);
                break;
            }
            else
            {
                deselectLines(startLine, -1, num_of_blocks);
                deselectLines(endLine, -1, num_of_blocks);
            }
        }
    }
    else if(flag1 == -1)
    {
        waveWidget->deselectAll();
    }

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
                        }
                    }
                }
            }

            waveWidget->replot();
        }
    }
}
