#include "SimpleRibbonButton.h"

SimpleRibbonButton::SimpleRibbonButton(QWidget* parent)
    : QToolButton(parent)
{
    QToolButton::setPopupMode(QToolButton::DelayedPopup);
    QToolButton::setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QToolButton::setMinimumSize(54, 54);
    QToolButton::setIconSize(QSize(32, 32));
    QToolButton::setAutoRaise(true);
    QToolButton::setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}
SimpleRibbonButton::~SimpleRibbonButton() {

}

void SimpleRibbonButton::SetText(const QString& text) {
    QToolButton::setText(text);
}

void SimpleRibbonButton::SetTooltip(const QString& tooltip) {
    QToolButton::setToolTip(tooltip);
}

void SimpleRibbonButton::SetIcon(const QIcon& icon) {
    QToolButton::setIcon(icon);
}

void SimpleRibbonButton::SetMenu(QMenu* menu) {
    QToolButton::setPopupMode(QToolButton::MenuButtonPopup);
    QToolButton::setMenu(menu);
}
