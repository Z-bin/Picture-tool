#ifndef BOTTOMBUTTONGROUP_H
#define BOTTOMBUTTONGROUP_H

#include <QAbstractButton>
#include <QGroupBox>
#include <QDebug>

/**
* @brief 工具箱按钮组
*
*/

class BottomButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit BottomButtonGroup(QWidget *parent = nullptr);

    void addButton(QAbstractButton *button);
};

#endif // BOTTOMBUTTONGROUP_H
