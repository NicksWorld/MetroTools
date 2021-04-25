#pragma once
#include <QTabWidget>

class SimpleRibbonTab;
class SimpleRibbonGroup;
class SimpleRibbonVBar;

class SimpleRibbon final : public QTabWidget {
    Q_OBJECT

public:
    SimpleRibbon(QWidget* parent = nullptr);
    ~SimpleRibbon() override;

    SimpleRibbonTab*    AddRibbonTab(const QString& title);
    SimpleRibbonTab*    GetRibbonTab(const QString& title);
    void                RemoveRibbonTab(const QString& title);

private:
    int                 FindRibbonTabIdx(const QString& title);
};
