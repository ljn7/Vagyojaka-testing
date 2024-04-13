#include "texttospeech.h"
#include "ui_texttospeech.h"

TextToSpeech::TextToSpeech(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TextToSpeech)
{
    ui->setupUi(this);
}

TextToSpeech::~TextToSpeech()
{
    delete ui;
}
