#include "mainwindow.h"

#include "bottombuttongroup.h"
#include "graphicsview.h"
#include "navigatorview.h"
#include "graphicsscene.h"

#include <QScreen>
#include <QDebug>
#include <QStyle>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QGraphicsOpacityEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置为无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setMinimumSize(710, 530);
    this->setWindowIcon(QIcon(":/icons/app-icon.svg"));

    // 与窗口透明度属性绑定
    m_fadeOutAnimation = new QPropertyAnimation(this, "windowOpacity");
    m_fadeOutAnimation->setDuration(300);
    m_fadeOutAnimation->setStartValue(1);
    m_fadeOutAnimation->setEndValue(0);
    // 与窗口geometry属性绑定
    m_floatUpAnimation = new QPropertyAnimation(this, "geometry");
    m_floatUpAnimation->setDuration(300);
    m_floatUpAnimation->setEasingCurve(QEasingCurve::OutCirc);

    m_exitAnimationGroup = new QParallelAnimationGroup(this);
    m_exitAnimationGroup->addAnimation(m_fadeOutAnimation);
    m_exitAnimationGroup->addAnimation(m_floatUpAnimation);
    connect(m_exitAnimationGroup, &QParallelAnimationGroup::finished,
            this, &MainWindow::close);

    GraphicsScene *scene = new GraphicsScene(this);

    m_graphicsView = new GraphicsView(this);
    m_graphicsView->setScene(scene);
    this->setCentralWidget(m_graphicsView);

    m_gv = new NavigatorView(this);
    m_gv->setFixedSize(220, 160);
    m_gv->setScene(scene);
    m_gv->setMainView(m_graphicsView);
    m_gv->fitInView(m_gv->sceneRect(), Qt::KeepAspectRatio);

    connect(m_graphicsView, &GraphicsView::navigatorViewRequired,
            this, [ = ](bool required, qreal angle) {
        m_gv->resetTransform();
        m_gv->rotate(angle);
        m_gv->fitInView(m_gv->sceneRect(), Qt::KeepAspectRatio);
        m_gv->setVisible(required);
    });

    connect(m_graphicsView, &GraphicsView::viewportRectChanged,
            m_gv, &NavigatorView::updateMainViewportRegion);


    m_closeButton = new QPushButton(m_graphicsView);
    m_closeButton->setFlat(true);
    m_closeButton->setFixedSize(50, 50);
    m_closeButton->setStyleSheet("QPushButton {"
                                 "background: transparent;"
                                 "}"
                                 "QPushButton:hover {"
                                 "background: red;"
                                 "}");

    connect(m_closeButton, &QPushButton::clicked,
            this, &MainWindow::closeWindow);

    m_bottomButtonGroup = new BottomButtonGroup(this);

    connect(m_bottomButtonGroup, &BottomButtonGroup::resetToOriginalBtnClicked,
            this, [=](){ m_graphicsView->resetScale();});
    connect(m_bottomButtonGroup, &BottomButtonGroup::zoomInBtnClicked,
            this, [=](){ m_graphicsView->zoomView(1.25);});
    connect(m_bottomButtonGroup, &BottomButtonGroup::zoomOutBtnClicked,
            this, [=](){ m_graphicsView->zoomView(0.75);});
    connect(m_bottomButtonGroup, &BottomButtonGroup::toggleCheckerboardBtnClicked,
            this, [=](){ m_graphicsView->toggleCheckerboard();});
    connect(m_bottomButtonGroup, &BottomButtonGroup::rotateRightBtnClicked,
            this, [=](){
        m_graphicsView->resetScale();
        m_graphicsView->rotateView(90);
        m_graphicsView->checkAndDoFitInView();
        m_gv->setVisible(false);
    });
    connect(m_bottomButtonGroup, &BottomButtonGroup::toggleWindowMaximum,
            this, [=](){
        if (isMaximized()) {
           showNormal();
        } else {
            showMaximized();
        }
    });


    m_btnGrpEffect = new QGraphicsOpacityEffect(this);
    m_bribViewEffect = new QGraphicsOpacityEffect(this);
    m_bottomButtonGroup->setGraphicsEffect(m_btnGrpEffect);
    m_gv->setGraphicsEffect(m_bribViewEffect);

    m_btnGrpOpacityAnimation = new QPropertyAnimation(m_btnGrpEffect, "opacity");
    m_btnGrpOpacityAnimation->setDuration(300);
    m_bribViewOpacityAnimation = new QPropertyAnimation(m_bribViewEffect, "opacity");
    m_bribViewOpacityAnimation->setDuration(300);

    m_btnGrpEffect->setOpacity(0);
    m_bribViewEffect->setOpacity(0);

    centerWindow();
}

MainWindow::~MainWindow()
{
}

void MainWindow::showUrls(const QList<QUrl> &urls)
{
    m_graphicsView->showFromUrlList(urls);
    m_gv->fitInView(m_gv->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::adjustWindowSizeBySceneRect()
{
    // 如果通过调整resize来调整缩放
    if (m_graphicsView->scaleFactor() < 1) {
        QSize screenSize = qApp->screenAt(QCursor::pos())->availableSize();
        QSize sceneSize = m_graphicsView->sceneRect().toRect().size();
        QSize sceneSizeWithMarigins = sceneSize + QSize(130, 125);
        if (screenSize.expandedTo(sceneSize) == screenSize) {
            // 通过增加窗口大小来显示图片(当图片大小小于窗口大小但是大于默认窗口大小时候)
            if (screenSize.expandedTo(sceneSize) == screenSize) {
                this->resize(sceneSizeWithMarigins);
            } else {
                this->resize(screenSize);
            }
            centerWindow();
            m_graphicsView->resetScale();
        } else {
            showMaximized();
        }
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    updateWidgetsPosition();
    return QMainWindow::showEvent(event);
}

void MainWindow::enterEvent(QEvent *event)
{
    m_btnGrpOpacityAnimation->stop();
    m_btnGrpOpacityAnimation->setStartValue(m_btnGrpEffect->opacity());
    m_btnGrpOpacityAnimation->setEndValue(1);
    m_btnGrpOpacityAnimation->start();


    m_bribViewOpacityAnimation->stop();
    m_bribViewOpacityAnimation->setStartValue(m_bribViewEffect->opacity());
    m_bribViewOpacityAnimation->setEndValue(1);
    m_bribViewOpacityAnimation->start();

    return QMainWindow::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    m_btnGrpOpacityAnimation->stop();
    m_btnGrpOpacityAnimation->setStartValue(m_btnGrpEffect->opacity());
    m_btnGrpOpacityAnimation->setEndValue(0);
    m_btnGrpOpacityAnimation->start();

    m_bribViewOpacityAnimation->stop();
    m_bribViewOpacityAnimation->setStartValue(m_bribViewEffect->opacity());
    m_bribViewOpacityAnimation->setEndValue(0);
    m_bribViewOpacityAnimation->start();

    return QMainWindow::leaveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_clickedOnWindow = true;
        m_oldMousePos = event->pos();
        qDebug() << m_oldMousePos;
        event->accept();
    }

    return QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_clickedOnWindow) {
        move(event->globalPos() - m_oldMousePos);
        event->accept();
    }

    return QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_clickedOnWindow = false;

    return QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_protectMode) {
        closeWindow();
    }

    return QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        m_graphicsView->zoomView(1.25);
    } else {
        m_graphicsView->zoomView(0.8);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    updateWidgetsPosition();

    return QMainWindow::resizeEvent(event);
}

void MainWindow::centerWindow()
{
    this->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    this->size(),
                    qApp->screenAt(QCursor::pos())->geometry()
                    )
                );
}

void MainWindow::closeWindow()
{
    m_floatUpAnimation->setStartValue(QRect(this->geometry().x(), this->geometry().y(), this->geometry().width(), this->geometry().height()));
    m_floatUpAnimation->setEndValue(QRect(this->geometry().x(), this->geometry().y()-80, this->geometry().width(), this->geometry().height()));
    m_exitAnimationGroup->start();
}

void MainWindow::updateWidgetsPosition()
{
    m_closeButton->move(width() - m_closeButton->width(), 0);
    m_bottomButtonGroup->move((width() - m_bottomButtonGroup->width()) / 2,
                              height() - m_bottomButtonGroup->height());
    m_gv->move(width() - m_gv->width(), height() - m_gv->height());
}

