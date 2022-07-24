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
    // 鼠标下方点作为改变视图大小的锚定点
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setStyleSheet("background-color: rgba(0, 0, 0, 220);"
                  "border-radius: 3px;");
    setAcceptDrops(true);
    setCheckerboardEnabled(false);
}

void GraphicsView::showFromUrlList(const QList<QUrl> &urlList)
{
    emit navigatorViewRequired(false, 0);
    // 拖动原始QQ图纸会触发此问题
    if (urlList.isEmpty()) {
        showText(tr("File url list is empty"));
        qDebug() << Q_FUNC_INFO << "File url list is empty";
        return;
    }

    QUrl url(urlList.first());
    QString filePath(url.toLocalFile());

    if (filePath.endsWith(".svg")) {
        showSvg(filePath);
    } else if (filePath.endsWith(".gif")) {
        showGif(filePath);
    } else {
       QImageReader imageReader(filePath);
       QImage::Format imageFormat = imageReader.imageFormat();
       if (imageFormat == QImage::Format_Invalid) {
           showText("File not is a valid image");
       } else {
           showImage(QPixmap::fromImageReader(&imageReader));
       }
    }
}

void GraphicsView::showImage(const QPixmap &pixmap)
{
    resetTransform();
    scene()->showImage(pixmap);
    checkAndDoFitInView();
}

void GraphicsView::showText(const QString &text)
{
    resetTransform();
    scene()->showText(text);
    checkAndDoFitInView();
}

void GraphicsView::showSvg(const QString &filepath)
{
    resetTransform();
    scene()->showSvg(filepath);
    checkAndDoFitInView();
}

void GraphicsView::showGif(const QString &filepath)
{
    resetTransform();
    scene()->showGif(filepath);
    checkAndDoFitInView();
}

GraphicsScene *GraphicsView::scene() const
{
    return qobject_cast<GraphicsScene*>(QGraphicsView::scene());
}

void GraphicsView::setScene(GraphicsScene *scene)
{
    return QGraphicsView::setScene(scene);
}

qreal GraphicsView::scaleFactor() const
{
    int angle = static_cast<int>(m_rotateAngle);
    if (angle == 0 || angle == 180) {
        return qAbs(transform().m11());
    } else {
        return qAbs(transform().m12());
    }
}

void GraphicsView::resetTransform()
{
    m_scaleFactor = 1;
    m_rotateAngle = 0;
    QGraphicsView::resetTransform();
}

void GraphicsView::zoomView(qreal scaleFactor)
{
    m_enableFitInView = false;
    m_scaleFactor *= scaleFactor;
    reapplyViewTransform();
    emit navigatorViewRequired(!isThingSmallerThanWindowWith(transform()), m_rotateAngle);
}

void GraphicsView::resetScale()
{
    m_scaleFactor = 1;
    reapplyViewTransform();
    emit navigatorViewRequired(!isThingSmallerThanWindowWith(transform()), m_rotateAngle);
}

void GraphicsView::rotateView(qreal rotateAngle)
{
    m_rotateAngle += rotateAngle;
    m_rotateAngle = static_cast<int>(m_rotateAngle) % 360;
    reapplyViewTransform();
}

void GraphicsView::checkAndDoFitInView()
{
    if (!isThingSmallerThanWindowWith(transform())) {
        m_enableFitInView = true;
        // 保证图像适应窗口
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
}

void GraphicsView::toggleCheckerboard()
{
    setCheckerboardEnabled(!m_checkerboardEnabled);
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (shouldIgnoreMousePressMoveEvent(event)) {
        event->ignore();
        // 在这里返回，否则如果我们设置一个 QGraphicsView::ScrollHandDrag 拖动模式，QMouseEvent 事件透明度将不起作用。
        return;
    }

    return QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (shouldIgnoreMousePressMoveEvent(event)) {
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
    event->ignore();
    return QGraphicsView::wheelEvent(event);
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    if (m_enableFitInView) {
        QTransform tf;
        tf.rotate(m_rotateAngle);
        if (isThingSmallerThanWindowWith(QTransform(tf)) && scaleFactor() >= 1) {
            // 当用户放大窗口时候，什么都不做
        } else {
            // 缩小窗口时候，仍旧保持图像适应窗口大小
            fitInView(sceneRect(), Qt::KeepAspectRatio);
        }
    } else {
        // 是否显示缩略图
        emit navigatorViewRequired(!isThingSmallerThanWindowWith(transform()), m_rotateAngle);
    }
    return QGraphicsView::resizeEvent(event);
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
        showFromUrlList(mimeData->urls());
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
    } else {
        showText("Not supported mimedata: " + mimeData->formats().first());
    }
}

void GraphicsView::paintEvent(QPaintEvent *event)
{
    if (event->rect() == this->rect() && !isThingSmallerThanWindowWith(transform())) {
        emit viewportRectChanged();
    }
    return QGraphicsView::paintEvent(event);
}

bool GraphicsView::isThingSmallerThanWindowWith(const QTransform &transform) const
{
    return rect().size().expandedTo(transform.mapRect(sceneRect()).size().toSize())
            == rect().size();
}

bool GraphicsView::shouldIgnoreMousePressMoveEvent(const QMouseEvent *event) const
{
    // 只是鼠标划过时候
    if (event->buttons() == Qt::NoButton) {
        return true;
    }

    if (isThingSmallerThanWindowWith(transform())) {
        return true;
    }

    QGraphicsItem *item = itemAt(event->pos());
    if (!item) {
        return true;
    }

    return false;
}

void GraphicsView::setCheckerboardEnabled(bool enabled)
{
    m_checkerboardEnabled = enabled;
    if (m_checkerboardEnabled) {
        QPixmap tilePixmap(0x20, 0x20);
        tilePixmap.fill(QColor(30, 30, 30, 100));
        QPainter tilePainter(&tilePixmap);
        QColor color(40, 40, 40, 100);
        tilePainter.fillRect(0, 0, 0x10, 0x10, color);
        tilePainter.fillRect(0x10, 0x10, 0x10, 0x10, color);
        tilePainter.end();

        setBackgroundBrush(tilePixmap);
    } else {
        setBackgroundBrush(Qt::transparent);
    }
}

void GraphicsView::reapplyViewTransform()
{
    QGraphicsView::resetTransform();
    scale(m_scaleFactor, m_scaleFactor);
    rotate(m_rotateAngle);
}
