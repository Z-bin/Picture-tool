#include "mainwindow.h"

#include "toolbutton.h"

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
#include <QMenu>
#include <QShortcut>

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
        m_gv->updateMainViewportRegion();
    });

    connect(m_graphicsView, &GraphicsView::viewportRectChanged,
            m_gv, &NavigatorView::updateMainViewportRegion);


    m_closeButton = new ToolButton(m_graphicsView);
    m_closeButton->setIcon(QIcon(":/icons/window-close"));
    m_closeButton->setIconSize(QSize(50, 50));

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

    m_bottomButtonGroup->setOpacity(0, false);
    m_gv->setOpacity(0, false);
    m_closeButton->setOpacity(0, false);

    QShortcut *quitAppShortCut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(quitAppShortCut, &QShortcut::activated, this, std::bind(&MainWindow::quitAppAction, this, false));

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
    QSize sceneSize = m_graphicsView->sceneRect().toRect().size();
    QSize sceneSizeWithMarigins = sceneSize + QSize(130, 125);
    // 如果通过调整resize来调整缩放
    if (m_graphicsView->scaleFactor() < 1 || size().expandedTo(sceneSizeWithMarigins) != size()) {
        QSize screenSize = qApp->screenAt(QCursor::pos())->availableSize();
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
    m_bottomButtonGroup->setOpacity(1);
    m_gv->setOpacity(1);
    m_closeButton->setOpacity(1);

    return QMainWindow::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    m_bottomButtonGroup->setOpacity(0);
    m_gv->setOpacity(0);
    m_closeButton->setOpacity(0);

    return QMainWindow::leaveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !isMaximized()) {
        m_clickedOnWindow = true;
        m_oldMousePos = event->pos();
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
    quitAppAction();

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

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu;
    QAction *protectMode = new QAction(tr("Protected mode"));
    connect(protectMode, &QAction::triggered, this, [=](){
       toggleProtectMode();
    });

    QAction *stayOnTopMode = new QAction(tr("Stay on top"));
    connect(stayOnTopMode, &QAction::triggered, this, [=](){
        toggleStayOnTop();
    });

    stayOnTopMode->setCheckable(true);
    stayOnTopMode->setCheckable(stayOnTop());

    protectMode->setCheckable(true);
    protectMode->setChecked(m_protectMode);

    QAction * helpAction = new QAction(tr("Help"));
    connect(helpAction, &QAction::triggered, this, [ = ](){
        QStringList sl {
            tr("Launch application with image file path as argument to load the file."),
            tr("Drag and drop image file onto the window is also supported."),
            "",
            tr("Context menu option explanation:"),
            (tr("Stay on top") + " : " + tr("Make window stay on top of all other windows.")),
            (tr("Protected mode") + " : " + tr("Avoid close window accidentally. (eg. by double clicking the window)"))
        };
        m_graphicsView->showText(sl.join('\n'));
    });


    menu->addAction(stayOnTopMode);
    menu->addAction(protectMode);
    menu->addSeparator();
    menu->addAction(helpAction);
    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();

    return QMainWindow::contextMenuEvent(event);
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

void MainWindow::toggleProtectMode()
{
    m_protectMode = !m_protectMode;
    m_closeButton->setVisible(!m_protectMode);
}

void MainWindow::toggleStayOnTop()
{
    setWindowFlag(Qt::WindowStaysOnTopHint, !stayOnTop());
    show();
}

bool MainWindow::stayOnTop()
{
    return windowFlags().testFlag(Qt::WindowStaysOnTopHint);
}

void MainWindow::quitAppAction(bool force)
{
    if (!m_protectMode || force) {
        closeWindow();
    }
}

