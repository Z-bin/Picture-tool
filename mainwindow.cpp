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
#include <QDir>
#include <QCollator>
#include <QClipboard>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置为无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setMinimumSize(350, 350);
    this->setWindowIcon(QIcon(":/icons/app-icon.svg"));
    this->setMouseTracking(true);

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

    connect(m_graphicsView, &GraphicsView::requestGallery,
            this, &MainWindow::loadGalleryBySingleLocalFile);

    m_closeButton = new ToolButton(true, m_graphicsView);
    m_closeButton->setIcon(QIcon(":/icons/window-close"));
    m_closeButton->setIconSize(QSize(50, 50));

    connect(m_closeButton, &QPushButton::clicked,
            this, &MainWindow::closeWindow);

    m_prevButton = new ToolButton(false, m_graphicsView);
    m_prevButton->setIcon(QIcon(":/icons/go-previous"));
    m_prevButton->setIconSize(QSize(75, 75));
    m_prevButton->setVisible(false);
    m_prevButton->setOpacity(0, false);
    m_nextButton = new ToolButton(false, m_graphicsView);
    m_nextButton->setIcon(QIcon(":/icons/go-next"));
    m_nextButton->setIconSize(QSize(75, 75));
    m_nextButton->setVisible(false);
    m_nextButton->setOpacity(0, false);

    connect(m_prevButton, &QAbstractButton::clicked,
            this, &MainWindow::galleryPrev);
    connect(m_nextButton, &QAbstractButton::clicked,
            this, &MainWindow::galleryNext);

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

    connect(this, &MainWindow::galleryLoaded, this, [this]() {
        m_prevButton->setVisible(isGalleryAvailable());
        m_nextButton->setVisible(isGalleryAvailable());
    });

    QShortcut *quitAppShortCut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(quitAppShortCut, &QShortcut::activated, this, std::bind(&MainWindow::quitAppAction, this, false));

    QShortcut *quitAppShortCut2 = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(quitAppShortCut2, &QShortcut::activated, this, std::bind(&MainWindow::quitAppAction, this, false));


    QShortcut * prevPictureShorucut = new QShortcut(QKeySequence(Qt::Key_PageUp), this);
    connect(prevPictureShorucut, &QShortcut::activated,
            this, &MainWindow::galleryPrev);

    QShortcut * nextPictureShorucut = new QShortcut(QKeySequence(Qt::Key_PageDown), this);
    connect(nextPictureShorucut, &QShortcut::activated,
            this, &MainWindow::galleryNext);

    QShortcut * fullscreenShorucut = new QShortcut(QKeySequence(QKeySequence::FullScreen), this);
    connect(fullscreenShorucut, &QShortcut::activated,
            this, &MainWindow::toggleFullscreen);

    centerWindow();
}

MainWindow::~MainWindow()
{
}

void MainWindow::showUrls(const QList<QUrl> &urls)
{
    if (!urls.isEmpty()) {
        if (urls.count() == 1) {
            m_graphicsView->showFileFromUrl(urls.first(), true);
        } else {
            m_graphicsView->showFileFromUrl(urls.first(), false);
            m_files = urls;
            m_currentFileIndex = 0;
        }
    } else {
        m_graphicsView->showText(tr("File url list is empty"));
        return;
    }
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
            QSize finalSize = (screenSize.expandedTo(sceneSizeWithMarigins) == screenSize) ?
                              sceneSizeWithMarigins : screenSize;
            this->resize(finalSize.expandedTo(this->sizeHint()));
            m_graphicsView->resetScale();
            centerWindow();
        } else {
            showMaximized();
        }
    }
}

QUrl MainWindow::currentImageFileUrl()
{
    if (m_currentFileIndex != -1) {
        return m_files.value(m_currentFileIndex);
    }
    return QUrl();
}

void MainWindow::clearGallery()
{
    m_currentFileIndex = -1;
    m_files.clear();
}

void MainWindow::loadGalleryBySingleLocalFile(const QString &path)
{
    QFileInfo info(path);
    QDir dir(info.path());
    QString currentFileName = info.fileName();
    QStringList entryList = dir.entryList({"*.jpg", "*.jpeg", ".jfif", "*.png", "*.gif", "*.svg", "*.bmp"},
                                          QDir::Files | QDir::NoSymLinks, QDir::NoSort);

    QCollator collator;
    collator.setNumericMode(true);

    std::sort(entryList.begin(), entryList.end(), collator);

    clearGallery();

    for (int i = 0; i < entryList.count(); i++) {
        const QString &oneEntry = entryList.at(i);
        m_files.append(QUrl::fromLocalFile(dir.absoluteFilePath(oneEntry)));
        if (oneEntry == currentFileName) {
            m_currentFileIndex = i;
        }
    }

    emit galleryLoaded();
}

void MainWindow::galleryPrev()
{
    int count = m_files.count();
    if (!isGalleryAvailable()) {
        return;
    }

    m_currentFileIndex = m_currentFileIndex - 1 < 0 ? count - 1 : m_currentFileIndex - 1;
    m_graphicsView->showFileFromUrl(m_files.at(m_currentFileIndex), false);
}

void MainWindow::galleryNext()
{
    int count = m_files.count();
    if (!isGalleryAvailable()) {
        return;
    }

    m_currentFileIndex = m_currentFileIndex + 1 == count ? 0 : m_currentFileIndex + 1;

    m_graphicsView->showFileFromUrl(m_files.at(m_currentFileIndex), false);
}

bool MainWindow::isGalleryAvailable()
{
    if (m_currentFileIndex < 0 || m_files.isEmpty() || m_currentFileIndex >= m_files.count()) {
        return false;
    }
    return true;
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
    m_prevButton->setOpacity(1);
    m_nextButton->setOpacity(1);

    return QMainWindow::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    m_bottomButtonGroup->setOpacity(0);
    m_gv->setOpacity(0);

    m_closeButton->setOpacity(0);
    m_prevButton->setOpacity(0);
    m_nextButton->setOpacity(0);

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
    QPoint numDegress = event->angleDelta() / 8;
    bool needZoom = false, zoomIn = false;
    // 在X11上QWheelEvent::pixelDelta()可能会失效
    if (!numDegress.isNull() && numDegress.y() != 0) {
        needZoom = true;
        zoomIn = numDegress.y() > 0;
    }

    if (needZoom) {
        m_graphicsView->zoomView(zoomIn ? 1.25 : 0.8);
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
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
    QMenu *copyMenu = new QMenu(tr("&Copy"));
    QUrl currentFileUrl = currentImageFileUrl();
    QImage clipboardImage;
    QUrl clipboardFileUrl;

    const QMimeData *clipboardData = QApplication::clipboard()->mimeData();
    if (clipboardData->hasImage()) {
        QVariant imageVariant(clipboardData->imageData());
        if (imageVariant.isValid()) {
            clipboardImage = qvariant_cast<QImage>(imageVariant);
        }
    } else if (clipboardData->hasText()) {
        QString clipboardText(clipboardData->text());
        if (clipboardText.startsWith("PICTURE:")) {
            QString maybeFileName(clipboardText.mid(9));
            if (QFile::exists(maybeFileName)) {
                clipboardFileUrl = QUrl::fromLocalFile(maybeFileName);
            }
        }
    }

    QAction *copyPixmap = new QAction(tr("Copy &Pixmap"));
    connect(copyPixmap, &QAction::triggered, this, [=]() {
        QClipboard *cb = QApplication::clipboard();
        cb->setPixmap(m_graphicsView->scene()->renderToPixmap());
    });

    QAction *copyFilePath = new QAction(tr("Copy &File Path"));
    connect(copyFilePath, &QAction::triggered, this, [=]() {
       QClipboard *cb =  QApplication::clipboard();
       cb->setText(currentFileUrl.toLocalFile());
    });

    copyMenu->addAction(copyPixmap);
    if (currentFileUrl.isValid()) {
        copyMenu->addAction(copyFilePath);
    }

    QAction *pasteImage = new QAction(tr("&Paste Image"));
    connect(pasteImage, &QAction::triggered, this, [=](){
        clearGallery();
        m_graphicsView->showImage(clipboardImage);
    });

    QAction *pasteImageFile = new QAction(tr("&Paste Image File"));
    connect(pasteImageFile, &QAction::triggered, this, [=](){
       m_graphicsView->showFileFromUrl(clipboardFileUrl, true);
    });

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

    if (copyMenu->actions().count() == 1) {
        menu->addActions(copyMenu->actions());
    } else {
        menu->addMenu(copyMenu);
    }

    if (!clipboardImage.isNull()) {
        menu->addAction(pasteImage);
    } else if (clipboardFileUrl.isValid()) {
        menu->addAction(pasteImageFile);
    }

    menu->addAction(stayOnTopMode);
    menu->addAction(protectMode);
    menu->addSeparator();
    menu->addAction(helpAction);
    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();

    return QMainWindow::contextMenuEvent(event);
}

QSize MainWindow::sizeHint() const
{
    return QSize(350, 350);
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
    m_prevButton->move(25, (height() - m_prevButton->height()) / 2);
    m_nextButton->move(width() - m_nextButton->width() - 25,
                       (height() - m_prevButton->height()) / 2);
    m_bottomButtonGroup->move((width() - m_bottomButtonGroup->width()) / 2,
                              height() - m_bottomButtonGroup->height());
    m_gv->move(width() - m_gv->width(), height() - m_gv->height());
}

void MainWindow::toggleProtectMode()
{
    m_protectMode = !m_protectMode;
    m_closeButton->setVisible(!m_protectMode);
    m_prevButton->setVisible(!m_protectMode);
    m_nextButton->setVisible(!m_protectMode);
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

void MainWindow::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

