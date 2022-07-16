#include "mainwindow.h"

#include "bottombuttongroup.h"
#include "graphicsview.h"
#include "graphicsscene.h"

#include <QScreen>
#include <QDebug>
#include <QStyle>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsTextItem>

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
        m_graphicsView->checkAndDoFitView();
    });


    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->screenAt(QCursor::pos())->geometry()
        )
    );

}

MainWindow::~MainWindow()
{
}

void MainWindow::showUrls(const QList<QUrl> &urls)
{
    m_graphicsView->showFromUrlList(urls);
}

void MainWindow::adjustWindowSizeBySceneRect()
{
    // 如果通过调整resize来调整缩放
    if (m_graphicsView->scaleFactor() < 1) {
        QSize screenSize = qApp->screenAt(QCursor::pos())->availableSize();
        QSize sceneSize = m_graphicsView->sceneRect().toRect().size();
        QSize sceneSizeWithMarigins = sceneSize + QSize(20, 20);
        if (screenSize.expandedTo(sceneSize) == screenSize) {
            // 通过增加窗口大小来显示图片(当图片大小小于窗口大小但是大于默认窗口大小时候)
            if (screenSize.expandedTo(sceneSize) == screenSize) {
                this->resize(sceneSizeWithMarigins);
            } else {
                this->resize(screenSize);
            }
            centerWindow();
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
    closeWindow();

    return QMainWindow::mouseDoubleClickEvent(event);
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
}

