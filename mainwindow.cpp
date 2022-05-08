#include "mainwindow.h"

#include "bottombuttongroup.h"
#include "graphicsview.h"

#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置为无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setMinimumSize(610, 410);

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

    QGraphicsScene *scene = new QGraphicsScene(this);
    QGraphicsTextItem *textItem = scene->addText("Hello, world!");
    textItem->setDefaultTextColor(QColor("White"));

    GraphicsView *test = new GraphicsView(this);
    test->setScene(scene);
    this->setCentralWidget(test);


    m_closeButton = new QPushButton(this);
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

}

MainWindow::~MainWindow()
{
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

