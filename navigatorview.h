#ifndef NAVIGATORVIEW_H
#define NAVIGATORVIEW_H

class GraphicsView;

#include <QGraphicsView>

class NavigatorView : public QGraphicsView
{
    Q_OBJECT
public:
    NavigatorView(QWidget *parent = nullptr);

    void setMainView(GraphicsView *mainView);

public slots:
    void updateMainViewportRegion();

private:
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseMoveEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event)        override;
    void paintEvent(QPaintEvent *event)        override;

    bool m_mouseDown = false;
    QPolygon m_viewportRegion;
    QGraphicsView *m_mainView = nullptr;
};

#endif // NAVIGATORVIEW_H
