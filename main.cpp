#include "mainwindow.h"
#include "graphicsview.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument("File List", QCoreApplication::translate("main", "File list."));
    parser.process(a);

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
    }

    return a.exec();
}
