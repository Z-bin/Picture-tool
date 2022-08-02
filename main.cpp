#include "mainwindow.h"
#include "graphicsview.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(QString("PineapplePictures_%1").arg(QLocale::system().name()), ":/i18n/");
    a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.addPositionalArgument("File List", QCoreApplication::translate("main", "File list."));
    parser.process(a);
    parser.addHelpOption();

    QStringList urlStrList = parser.positionalArguments();
    QList<QUrl> urlList;
    for (const QString &str : urlStrList) {
        QUrl url = QUrl::fromLocalFile(str);
        if (url.isValid()) {
            urlList.append(url);
        }
    }

    MainWindow w;
    w.show();

    if (!urlList.empty()) {
        w.showUrls(urlList);
        w.adjustWindowSizeBySceneRect();
    }

    return a.exec();
}
