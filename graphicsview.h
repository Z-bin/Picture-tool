#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QUrl>

class GraphicsScene;
class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget *parent = nullptr);

    void showFileFromUrl(const QUrl &url, bool doRequestGallery = false);

    void showImage(const QPixmap &pixmap);
    void showImage(const QImage &image);
    void showText(const QString &text);
    void showSvg(const QString &filepath);
    void showGif(const QString &filepath);

    GraphicsScene * scene() const;
    void setScene(GraphicsScene *scene);

    qreal scaleFactor() const;

    void resetTransform();
    void zoomView(qreal scaleFactor);
    void resetScale();
    void rotateView(qreal rotateAngle);

    void checkAndDoFitInView();

signals:
   void navigatorViewRequired(bool required, qreal angle);
   void viewportRectChanged();
   void requestGallery(const QString &filePath);

public slots:
    void toggleCheckerboard();

private:
    void mousePressEvent(QMouseEvent *event)      override;
    void mouseMoveEvent(QMouseEvent *event)       override;
    void mouseReleaseEvent(QMouseEvent *event)    override;
    void wheelEvent(QWheelEvent *event)           override;
    void resizeEvent(QResizeEvent *event)         override;

    void dragEnterEvent(QDragEnterEvent *event)   override;
    void dragMoveEvent(QDragMoveEvent *event)     override;
    void dropEvent(QDropEvent *event)             override;

    /*!
     * @brief 图片是否大于窗口大小
     */
    bool isThingSmallerThanWindowWith(const QTransform &transform) const;
    bool shouldIgnoreMousePressMoveEvent(const QMouseEvent *event) const;

    void setCheckerboardEnabled(bool enabled);

    void resetWithScaleAndRotate(qreal scaleFactor, qreal rotateAngle);

    bool m_enableFitInView     = false;
    bool m_checkerboardEnabled = false;

    qreal m_rotateAngle = 0;
};

#endif // GRAPHICSVIEW_H
