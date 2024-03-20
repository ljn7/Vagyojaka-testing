#include "videowidget.h"
#include "qmimedata.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <iostream>

VideoWidget::VideoWidget(QWidget *parent)
    : QVideoWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAcceptDrops(true);
}

void VideoWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        setFullScreen(false);
        event->accept();
    } else if (event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt) {
        setFullScreen(!isFullScreen());
        event->accept();
    } else {
        QVideoWidget::keyPressEvent(event);
    }
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    setFullScreen(!isFullScreen());
    event->accept();
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    QVideoWidget::mousePressEvent(event);
}

void VideoWidget::dragEnterEvent(QDragEnterEvent *event) {
    QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*>(event);
    if (dragEnterEvent->mimeData()->hasUrls()) {
        dragEnterEvent->acceptProposedAction();
    }
}

void VideoWidget::dragMoveEvent(QDragMoveEvent *event) {
    event->accept();
    event->acceptProposedAction();
}
void VideoWidget::dropEvent(QDropEvent *event)  {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QUrl* url = new QUrl(mimeData->urls().first());
        std::cerr << "dropped in vw";
        emit droppedFile(url);
    }
    event->acceptProposedAction();
}
