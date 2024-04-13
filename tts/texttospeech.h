#ifndef TEXTTOSPEECH_H
#define TEXTTOSPEECH_H

#include <QWidget>

namespace Ui {
class TextToSpeech;
}

class TextToSpeech : public QWidget
{
    Q_OBJECT

public:
    explicit TextToSpeech(QWidget *parent = nullptr);
    ~TextToSpeech();

private:
    Ui::TextToSpeech *ui;
};

#endif // TEXTTOSPEECH_H
