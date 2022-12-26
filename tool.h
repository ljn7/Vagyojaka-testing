#pragma once

#include <QMainWindow>
#include "mediaplayer/mediaplayer.h"
#include "editor/texteditor.h"
#include<QThread>
#include<QtConcurrent/QtConcurrent>
#include "./transcriptgenerator.h"
QT_BEGIN_NAMESPACE
    namespace Ui { class Tool; }
QT_END_NAMESPACE

    class Tool final : public QMainWindow
{
    Q_OBJECT

        public:
                 explicit Tool(QWidget *parent = nullptr);
    ~Tool() final;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void handleMediaPlayerError();
    void createKeyboardShortcutGuide();
    void changeFont();
    void changeFontSize(int change);
    void transliterationSelected(QAction* action);

    void on_Upload_and_generate_Transcript_triggered();
    //    void on_editor_openTranscript_triggered();

    void on_btn_translate_clicked();

    void on_editor_openTranscript_triggered();


private:
    void setFontForElements();
    void setTransliterationLangCodes();

    MediaPlayer *player = nullptr;
    Ui::Tool *ui;
    QFont font;
    QMap<QString, QString> m_transliterationLang;

};
