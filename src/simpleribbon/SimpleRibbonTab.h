#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>

class SimpleRibbonGroup;

class SimpleRibbonTab final : public QWidget {
    Q_OBJECT

public:
    SimpleRibbonTab(QWidget* parent = nullptr);
    ~SimpleRibbonTab() override;

    SimpleRibbonGroup*  AddRibbonGroup(const QString& title);
    SimpleRibbonGroup*  FindRibbonGroup(const QString& title);
    void                RemoveRibbonGroup(const QString& title);

private:
    QGridLayout*        mGridLayout;
    QHBoxLayout*        mHorizontalLayout;
};
