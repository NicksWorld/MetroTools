#pragma once
#include <QWidget>
#include <QVBoxLayout>

class SimpleRibbonVBar final : public QWidget {
    Q_OBJECT

public:
    SimpleRibbonVBar(QWidget* parent = nullptr);
    ~SimpleRibbonVBar() override;

    void    AddWidget(QWidget* widget);

private:
    QVBoxLayout*    mLayout;
};
