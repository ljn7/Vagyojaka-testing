#pragma once

#include "qitemselectionmodel.h"
#include "tts/customdelegates.h"
#include <QWidget>
#include <QUrl>
#include <QSettings>
#include <memory>

namespace Ui {
class TTSAnnotator;
}

class LazyLoadingModel;
class QTableView;

class TTSAnnotator : public QWidget
{
    Q_OBJECT

public:
    explicit TTSAnnotator(QWidget *parent = nullptr);
    ~TTSAnnotator();
    void openTTSTranscript();

private slots:
    void on_saveAsTableButton_clicked();
    void on_InsertRowButton_clicked();
    void on_deleteRowButton_clicked();
    void on_saveTableButton_clicked();
    void on_actionOpen_triggered();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onCellClicked(const QModelIndex &index);

private:
    void parseXML();
    void setupUI();
    void save();
    void saveAs();
    void saveToFile(const QString& fileName);
    void insertRow();
    void deleteRow();

    Ui::TTSAnnotator* ui;
    QTableView* tableView;
    std::unique_ptr<LazyLoadingModel> m_model;
    QUrl fileUrl;
    QString xmlDirectory;
    QSettings* settings;
    QStringList supportedFormats;
    AudioPlayerDelegate* m_audioPlayerDelegate;
};
