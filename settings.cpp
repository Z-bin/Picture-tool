#include "settings.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

Settings *Settings::m_settings_instance = nullptr;

Settings *Settings::instance()
{
    if (!m_settings_instance) {
        m_settings_instance = new Settings;
    }
    return m_settings_instance;
}

bool Settings::alwaysOnTop()
{
    return m_qsettings->value("always_on_top").toBool();
}

DoubleClickBehavior Settings::doubleClickBehavior()
{
    QString result = m_qsettings->value("double_click_behavior", "close").toString().toLower();
    return stringToDoubleClickBehavior(result);
}

QString Settings::doubleClickBehaviorToString(DoubleClickBehavior dcb)
{
    static QMap<DoubleClickBehavior, QString> _map {
        {ActionCloseWindow,   "close"},
        {ActionMaximizeWindow, "maximize"},
        {ActionDoNothing,      "ignore"}
    };

    return _map.value(dcb, "close");
}

DoubleClickBehavior Settings::stringToDoubleClickBehavior(QString str)
{
    static QMap<QString, DoubleClickBehavior> _map {
        {"close",    ActionCloseWindow},
        {"maximize", ActionMaximizeWindow},
        {"ignore",   ActionDoNothing}
    };

    return _map.value(str, ActionCloseWindow);
}

Settings::Settings() : QObject(qApp)
{
    QString configPath;

    QString portableConfigDirPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("data");
    QFileInfo portableConfigDirInfo(portableConfigDirPath);
    if (portableConfigDirInfo.exists() && portableConfigDirInfo.isDir() && portableConfigDirInfo.isWritable()) {
        // we can use it.
        configPath = portableConfigDirPath;
    }

    // %LOCALAPPDATA% under Windows.
    if (configPath.isEmpty()) {
        configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    }

    m_qsettings = new QSettings(QDir(configPath).absoluteFilePath("config.ini"), QSettings::IniFormat, this);
}
