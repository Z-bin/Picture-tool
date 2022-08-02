#ifndef BOTTOMBUTTONGROUP_H
#define BOTTOMBUTTONGROUP_H

#include <QAbstractButton>
#include <QGroupBox>
#include <QDebug>

class OpacityHelper;

/**
* @brief 工具箱按钮组
*
*/

class BottomButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit BottomButtonGroup(QWidget *parent = nullptr);

    void setOpacity(qreal opacity, bool animated = true);
    void addButton(QAbstractButton *button);

signals:
    void resetToOriginalBtnClicked();
    void toggleWindowMaximum();
    void zoomInBtnClicked();
    void zoomOutBtnClicked();
    void toggleCheckerboardBtnClicked();
    void rotateRightBtnClicked();

private:
    OpacityHelper *m_opacityHelper;
};

#endif // BOTTOMBUTTONGROUP_H
