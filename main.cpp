#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QTranslator>
#include <QUrl>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    QString qmDir;
    qmDir = QT_STRINGIFY(QM_FILE_INSTALL_DIR);
    translator.load(QString("PineapplePictures_%1").arg(QLocale::system().name()), qmDir);
    a.installTranslator(&translator);

    // parse commandline arguments
    QCommandLineParser parser;
    parser.addPositionalArgument("File list", QCoreApplication::translate("main", "File list."));
    parser.addHelpOption();

    parser.process(a);

    QStringList urlStrList = parser.positionalArguments();
    QList<QUrl> urlList;
    for (const QString & str : urlStrList) {
        QUrl url = QUrl::fromLocalFile(str);
        if (url.isValid()) {
            urlList.append(url);
        }
    }

    MainWindow w;
    w.show();

    if (!urlList.isEmpty()) {
        w.showUrls(urlList);
        w.adjustWindowSizeBySceneRect();
    }

    return a.exec();
}
