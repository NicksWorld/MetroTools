#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class SimpleRibbonGroup final : public QWidget {
    Q_OBJECT

public:
    SimpleRibbonGroup(QWidget* parent = nullptr);
    ~SimpleRibbonGroup() override;

    void            SetTitle(const QString& title);
    QString         GetTitle() const;

    void            AddWidget(QWidget* widget);

private:
    void            SetupLayout();

private:
    QGridLayout*    mGridLayout;
    QVBoxLayout*    mVerticalLayout;
    QHBoxLayout*    mHorizontalLayout;
    QLabel*         mTitleLabel;
    QFrame*         mVerticalLine;
};
