#ifndef TTSROW_H
#define TTSROW_H
#include "qstring.h"

struct TTSRow {
    QString words;
    QString not_pronunced_properly_1;
    int sound_quality_1;
    int tts_quality;

    int new_tts_quality_1;
    QString new_tts_words_not_pronunced_properly_1;
    int new_tts_sound_quality_1;

    int new_tts_quality_2;
    QString new_tts_words_not_pronunced_properly_2;
    int new_tts_sound_quality_2;
};

#endif // TTSBLOCK_H
