#pragma once
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class Frame; }
QT_END_NAMESPACE

class ImageInfoPanel : public QFrame {
    Q_OBJECT

public:
    ImageInfoPanel(QWidget* parent = nullptr);
    ~ImageInfoPanel();

    void    SetCompressionText(const QString& text);
    void    SetWidthText(const QString& text);
    void    SetHeightText(const QString& text);
    void    SetMipsText(const QString& text);

private:
    Ui::Frame*  ui;
};
