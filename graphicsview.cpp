#include "graphicsview.h"

#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView (parent)
{
    // 设置拖拽为手形拖拽
    setDragMode(QGraphicsView::ScrollHandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: rgba(0, 0, 0, 180);"
                  "border-radius: 3px;");
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
