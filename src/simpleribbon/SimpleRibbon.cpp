#include "SimpleRibbon.h"
#include "SimpleRibbonTab.h"

SimpleRibbon::SimpleRibbon(QWidget* parent)
    : QTabWidget(parent)
{
    QTabWidget::setAutoFillBackground(true);
    QTabWidget::setMovable(false);
    QTabWidget::setTabsClosable(false);
}
SimpleRibbon::~SimpleRibbon() {
}


SimpleRibbonTab* SimpleRibbon::AddRibbonTab(const QString& title) {
    SimpleRibbonTab* newTab = new SimpleRibbonTab();
    newTab->setObjectName(title);
    QTabWidget::addTab(newTab, title);
    return newTab;
}

SimpleRibbonTab* SimpleRibbon::GetRibbonTab(const QString& title) {
    SimpleRibbonTab* result = nullptr;

    const int idx = this->FindRibbonTabIdx(title);
    if (-1 != idx) {
        result = static_cast<SimpleRibbonTab*>(QTabWidget::widget(idx));
    }

    return result;
}

void SimpleRibbon::RemoveRibbonTab(const QString& title) {
    const int idx = this->FindRibbonTabIdx(title);
    if (-1 != idx) {
        QTabWidget::removeTab(idx);
    }
}

void SimpleRibbon::EnableRibbonTab(const QString& title, const bool enable) {
    const int idx = this->FindRibbonTabIdx(title);
    if (-1 != idx) {
        QTabWidget::setTabEnabled(idx, enable);
    }
}


int SimpleRibbon::FindRibbonTabIdx(const QString& title) {
    int result = -1;

    QString lowerTitle = title.toLower();
    for (int i = 0, end = QTabWidget::count(); i < end; ++i) {
        if (QTabWidget::tabText(i).toLower() == lowerTitle) {
            result = i;
            break;
        }
    }

    return result;
}
