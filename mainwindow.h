#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShowEvent>


QT_BEGIN_NAMESPACE
class QGraphicsOpacityEffect;
class QGraphicsView;
QT_END_NAMESPACE

class GraphicsView;
class NavigatorView;
class BottomButtonGroup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void showUrls(const QList<QUrl> &urls);
    void adjustWindowSizeBySceneRect();

protected slots:
    void showEvent(QShowEvent *event)              override;
    void enterEvent(QEvent *event)                 override;
    void leaveEvent(QEvent *event)                 override;
    void mousePressEvent(QMouseEvent *event)       override;
    void mouseMoveEvent(QMouseEvent *event)        override;
    void mouseReleaseEvent(QMouseEvent *event)     override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event)            override;
    void resizeEvent(QResizeEvent *event)          override;

    void centerWindow();
    void closeWindow();
    void updateWidgetsPosition();

private:
    QPoint                   m_oldMousePos;
    QGraphicsOpacityEffect  *m_btnGrpEffect;
    QGraphicsOpacityEffect  *m_bribViewEffect;
    QPropertyAnimation      *m_btnGrpOpacityAnimation;
    QPropertyAnimation      *m_bribViewOpacityAnimation;
    QPropertyAnimation      *m_fadeOutAnimation;
    QPropertyAnimation      *m_floatUpAnimation;
    QParallelAnimationGroup *m_exitAnimationGroup;
    QPushButton             *m_closeButton;
    GraphicsView            *m_graphicsView;
    NavigatorView           *m_gv;

    BottomButtonGroup       *m_bottomButtonGroup;

    bool                     m_clickedOnWindow = false;
};
#endif // MAINWINDOW_H
