#include "graphicsview.h"

#include "graphicsscene.h"

#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QMimeData>
#include <QImageReader>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView (parent)
{
    // 设置拖拽为手形拖拽
    setDragMode(QGraphicsView::ScrollHandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: rgba(0, 0, 0, 180);"
                  "border-radius: 3px;");
    setAcceptDrops(true);
}

void GraphicsView::showImage(const QPixmap &pixmap)
{
    scene()->showImage(pixmap);
}

void GraphicsView::showText(const QString &text)
{
    scene()->showText(text);
}

GraphicsScene *GraphicsView::scene() const
{
    return qobject_cast<GraphicsScene*>(QGraphicsView::scene());
}

void GraphicsView::setScene(GraphicsScene *scene)
{
    return QGraphicsView::setScene(scene);
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());

    if (!item) {
        event->ignore();
        // 在这里返回，否则如果我们设置一个 QGraphicsView::ScrollHandDrag 拖动模式，QMouseEvent 事件透明度将不起作用。
        return;
    }

    qDebug() << item;
    return QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());
    if (!item) {
        event->ignore();
    }

    return QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());
    if (!item) {
        event->ignore();
    }

    return QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        scale(1.25, 1.25);
    } else {
        scale(0.8, 0.8);
    }
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasImage() || event->mimeData()->hasText()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }

    qDebug() << event->mimeData() << "Drag Enter Event"
             << event->mimeData()->hasUrls() << event->mimeData()->hasImage()
             << event->mimeData()->formats() << event->mimeData()->hasFormat("text/uri-list");

    return QGraphicsView::dragEnterEvent(event);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    // 默认情况下，如果光标下没有 QGraphicsItem，QGraphicsView/Scene 将忽略该动作。(无法进行拖拽)
    // 我们实际上并不关心并且希望保持拖动事件原样，所以这里什么都不做。
    Q_UNUSED(event);
}

void GraphicsView::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        qDebug() << Q_FUNC_INFO << "this is url";
        QUrl url(mimeData->urls().first());
        QImageReader imageReader(url.toLocalFile());
        QImage::Format imageFormat = imageReader.imageFormat();
        if (imageFormat == QImage::Format_Invalid) {
            showText("File is not a valid image");
        } else {
            showImage(QPixmap::fromImageReader(&imageReader));
        }
    } else if (mimeData->hasImage()) {
        qDebug() << Q_FUNC_INFO << "this is image";
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        QPixmap pixmap = QPixmap::fromImage(img);
        if (pixmap.isNull()) {
            showText("Image data is invalid");
        } else {
            showImage(pixmap);
        }
    } else if (mimeData->hasText()) {
        showText(mimeData->text());
    }
}
