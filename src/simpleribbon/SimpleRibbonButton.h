#pragma once
#include <QToolButton>

class SimpleRibbonButton final : public QToolButton {
    Q_OBJECT

public:
    SimpleRibbonButton(QWidget* parent = nullptr);
    ~SimpleRibbonButton() override;

    void    SetText(const QString& text);
    void    SetTooltip(const QString& tooltip);
    void    SetIcon(const QIcon& icon);
    void    SetMenu(QMenu* menu);
};
