#include "ttsannotator.h"
#include "ui_ttsannotator.h"
#include "lazyloadingmodel.h"
#include "customdelegates.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

TTSAnnotator::TTSAnnotator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TTSAnnotator)
    , m_model(std::make_unique<LazyLoadingModel>())
{
    ui->setupUi(this);
    tableView = ui->tableView;
    tableView->setModel(m_model.get());

    setupUI();

    QString iniPath = QApplication::applicationDirPath() + "/" + "config.ini";
    settings = new QSettings(iniPath, QSettings::IniFormat);

    this->supportedFormats = {
        "xml Files (*.xml)",
        "All Files (*)"
    };
}

TTSAnnotator::~TTSAnnotator() = default;

void TTSAnnotator::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    for (const QModelIndex &index : deselected.indexes()) {
        if (index.column() == 0) { // Assuming audio player is in the first column
            tableView->closePersistentEditor(index);
        }
    }
    for (const QModelIndex &index : selected.indexes()) {
        if (index.column() == 0) { // Assuming audio player is in the first column
            tableView->openPersistentEditor(index);
        }
    }
}

void TTSAnnotator::setupUI()
{

    // Set up the model
    m_model = std::make_unique<LazyLoadingModel>(this);
    tableView->setModel(m_model.get());

    // Set headers for the model
    m_model->setHorizontalHeaderLabels({
        "Audios", "Transcript", "Mispronounced words", "Tags", "Sound Quality", "TTS Quality"
    });

    // Set up delegates
    m_audioPlayerDelegate = new AudioPlayerDelegate(xmlDirectory, this);
    tableView->setItemDelegateForColumn(0, m_audioPlayerDelegate);
    tableView->setItemDelegateForColumn(4, new ComboBoxDelegate(1, 5, this));
    tableView->setItemDelegateForColumn(5, new ComboBoxDelegate(0, 1, this));

    // Set up table view properties
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked |
                               QAbstractItemView::EditKeyPressed |
                               QAbstractItemView::AnyKeyPressed);

    // Set up header properties
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Enable sorting
    tableView->setSortingEnabled(true);

    // Set up connections
    connect(tableView, &QTableView::clicked, this, &TTSAnnotator::onCellClicked);

    // Set up button connections
    connect(ui->InsertRowButton, &QPushButton::clicked, this, &TTSAnnotator::insertRow);
    connect(ui->deleteRowButton, &QPushButton::clicked, this, &TTSAnnotator::deleteRow);
    connect(ui->saveAsTableButton, &QPushButton::clicked, this, &TTSAnnotator::saveAs);
    connect(ui->saveTableButton, &QPushButton::clicked, this, &TTSAnnotator::save);

    // Set initial focus
    tableView->setFocus();

    // Resize rows and columns to content
    tableView->resizeRowsToContents();
    tableView->resizeColumnsToContents();
}

void TTSAnnotator::openTTSTranscript()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open File"));
    fileDialog.setNameFilters(supportedFormats);

    if(settings->value("annotatorTranscriptDir").toString().isEmpty())
        fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).value(0, QDir::homePath()));
    else
        fileDialog.setDirectory(settings->value("annotatorTranscriptDir").toString());

    if (fileDialog.exec() == QDialog::Accepted) {
        m_model->clear();

        fileUrl = fileDialog.selectedUrls().constFirst();
        xmlDirectory = QFileInfo(fileUrl.toLocalFile()).absolutePath();
        if (m_audioPlayerDelegate)
            m_audioPlayerDelegate->setBaseDir(xmlDirectory);
        parseXML();

        QFileInfo filedir(fileUrl.toLocalFile());
        QString dirInString = filedir.dir().path();
        settings->setValue("annotatorTranscriptDir", dirInString);
    }
}

void TTSAnnotator::parseXML()
{
    QFile file(fileUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open file: %1").arg(file.errorString()));
        return;
    }

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        if (xmlReader.isStartElement() && xmlReader.name() == QString("row")) {
            TTSRow row;
            while (!(xmlReader.isEndElement() && xmlReader.name() == QString("row"))) {
                xmlReader.readNext();
                if (xmlReader.isStartElement()) {
                    QStringView elementName = xmlReader.name();
                    xmlReader.readNext();
                    QStringView text = xmlReader.text();
                    if (elementName == QString("words")) {
                        row.words = text.toString();
                    } else if (elementName == QString("not-pronounced-properly")) {
                        row.not_pronounced_properly = text.toString();
                    } else if (elementName == QString("sound-quality")) {
                        row.sound_quality = text.toInt();
                    } else if (elementName == QString("tts-quality")) {
                        row.tts_quality = text.toInt();
                    } else if (elementName == QString("audio-filename")) {
                        row.audioFileName = text.toString();
                    } else if (elementName == QString("tag")) {
                        row.tag = text.toString();
                    }
                }
            }
            m_model->addRow(row);
        }
        xmlReader.readNext();
    }
    file.close();
    if (xmlReader.hasError()) {
        QMessageBox::warning(this, tr("XML Error"), tr("Error parsing XML: %1").arg(xmlReader.errorString()));
    }
}

void TTSAnnotator::save()
{
    if (fileUrl.isEmpty()) {
        saveAs();
    } else {
        saveToFile(fileUrl.toLocalFile());
    }
}

void TTSAnnotator::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), xmlDirectory, tr("XML Files (*.xml)"));
    if (!fileName.isEmpty()) {
        fileUrl = QUrl::fromLocalFile(fileName);
        xmlDirectory = QFileInfo(fileName).absolutePath();
        saveToFile(fileName);
    }
}

void TTSAnnotator::saveToFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open file for writing: %1").arg(file.errorString()));
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("transcript");

    const auto& rows = m_model->rows();
    for (const auto& row : rows) {
        xmlWriter.writeStartElement("row");
        xmlWriter.writeTextElement("words", row.words);
        xmlWriter.writeTextElement("not-pronounced-properly", row.not_pronounced_properly);
        xmlWriter.writeTextElement("sound-quality", QString::number(row.sound_quality));
        xmlWriter.writeTextElement("tts-quality", QString::number(row.tts_quality));
        xmlWriter.writeTextElement("audio-filename", row.audioFileName);
        xmlWriter.writeTextElement("tag", row.tag);
        xmlWriter.writeEndElement(); // row
    }

    xmlWriter.writeEndElement(); // transcript
    xmlWriter.writeEndDocument();

    file.close();

    if (file.error() != QFile::NoError) {
        QMessageBox::warning(this, tr("Save Error"), tr("Error occurred while saving the file: %1").arg(file.errorString()));
    } else {
        QMessageBox::information(this, tr("Save Successful"), tr("File saved successfully."));
    }
}

void TTSAnnotator::insertRow()
{
    m_model->insertRow(m_model->rowCount());
}

void TTSAnnotator::deleteRow()
{
    QModelIndex currentIndex = tableView->currentIndex();
    if (currentIndex.isValid()) {
        m_model->removeRow(currentIndex.row());
    }
}

void TTSAnnotator::on_saveAsTableButton_clicked()
{
    saveAs();
}

void TTSAnnotator::on_InsertRowButton_clicked()
{
    insertRow();
}

void TTSAnnotator::on_deleteRowButton_clicked()
{
    deleteRow();
}

void TTSAnnotator::on_saveTableButton_clicked()
{
    save();
}

void TTSAnnotator::on_actionOpen_triggered()
{
    openTTSTranscript();
}

void TTSAnnotator::onCellClicked(const QModelIndex &index)
{
    if (index.column() == 0) {  // Assuming audio player is in the first column
        tableView->openPersistentEditor(index);
    }
}
