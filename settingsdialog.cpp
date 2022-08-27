#include "settingsdialog.h"

#include "settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QStringListModel>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_stayOntop(new QCheckBox)
    , m_doubleClickBehavior(new QComboBox)
{
    QFormLayout *settingsForm = new QFormLayout(this);

    static QMap<DoubleClickBehavior, QString> _map {
        { ActionDoNothing,      tr("Do nothing")},
        { ActionCloseWindow,    tr("Close the window")},
        { ActionMaximizeWindow, tr("Toggle maximize")}
    };

    QStringList dropDown;
    for (int dcb = ActionStart; dcb <= ActionEnd; dcb++) {
        dropDown.append(_map.value(static_cast<DoubleClickBehavior>(dcb)));
    }

    settingsForm->addRow(tr("Stay on top when start-up"), m_stayOntop);
    settingsForm->addRow(tr("Double-click behavior"), m_doubleClickBehavior);

    m_stayOntop->setChecked(Settings::instance()->stayOnTop());
    m_doubleClickBehavior->setModel(new QStringListModel(dropDown));
    DoubleClickBehavior dcb = Settings::instance()->doubleClickBehavior();
    m_doubleClickBehavior->setCurrentIndex(static_cast<int>(dcb));

    connect(m_stayOntop, &QCheckBox::stateChanged, this, [ = ](int state){
        Settings::instance()->setStayOnTop(state == Qt::Checked);
    });

    connect(m_doubleClickBehavior, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        Settings::instance()->setDoubleClickBehavior(static_cast<DoubleClickBehavior>(index));
    });

    setMinimumSize(200, 50);
}

SettingsDialog::~SettingsDialog()
{

}
