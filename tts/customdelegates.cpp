#include "customdelegates.h"
#include "audioplayer/audioplayerwidget.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include "qdir"

AudioPlayerDelegate::AudioPlayerDelegate(const QString& baseDir, QObject* parent)
    : QStyledItemDelegate(parent), m_baseDir(baseDir)
{
}

AudioPlayerDelegate::~AudioPlayerDelegate()
{
    qDeleteAll(m_activeEditors);
}

QWidget* AudioPlayerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)

    // Stop all other players
    stopAllPlayers();

    // Clean up unused editors
    cleanupUnusedEditors();

    // Create new editor if it doesn't exist
    if (!m_activeEditors.contains(index)) {
        QString fileName = index.model()->data(index, Qt::EditRole).toString();
        QString filePath = m_baseDir + QDir::separator() + fileName;
        AudioPlayerWidget* editor = new AudioPlayerWidget(filePath, parent);
        editor->setAutoFillBackground(true);
        m_activeEditors[index] = editor;
    }

    m_lastPlayingIndex = index;
    return m_activeEditors[index];
}

void AudioPlayerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
    // We don't need to set editor data here because we're creating a new AudioPlayerWidget
    // with the correct file path in createEditor
}

void AudioPlayerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    AudioPlayerWidget* audioPlayer = qobject_cast<AudioPlayerWidget*>(editor);
    if (audioPlayer) {
        QString fileName = audioPlayer->getAudioFileName(true);
        model->setData(index, fileName, Qt::EditRole);
    }
}

void AudioPlayerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void AudioPlayerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (m_activeEditors.contains(index)) {
        // If there's an active editor for this index, don't paint anything
        return;
    }

    // Otherwise, paint the default text
    QStyledItemDelegate::paint(painter, option, index);
}

void AudioPlayerDelegate::stopAllPlayers() const
{
    for (AudioPlayerWidget* player : m_activeEditors) {
        player->stop();  // Assuming AudioPlayerWidget has a stop() method
    }
}

void AudioPlayerDelegate::cleanupUnusedEditors() const
{
    QMutableMapIterator<QModelIndex, AudioPlayerWidget*> i(m_activeEditors);
    while (i.hasNext()) {
        i.next();
        if (i.key() != m_lastPlayingIndex) {
            delete i.value();
            i.remove();
        }
    }
}

void AudioPlayerDelegate::setBaseDir(QString pBaseDir)
{
    m_baseDir = pBaseDir;
}

ComboBoxDelegate::ComboBoxDelegate(int min, int max, QObject* parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max)
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox* editor = new QComboBox(parent);
    for (int i = m_min; i <= m_max; ++i) {
        editor->addItem(QString::number(i));
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        comboBox->setCurrentIndex(value - m_min);
    }
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        model->setData(index, comboBox->currentText().toInt(), Qt::EditRole);
    }
}
