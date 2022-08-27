#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class QCheckBox;
class QComboBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    QCheckBox *m_stayOntop = nullptr;
    QComboBox *m_doubleClickBehavior = nullptr;
};

#endif // SETTINGSDIALOG_H
