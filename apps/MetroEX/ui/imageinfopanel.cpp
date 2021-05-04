#include "imageinfopanel.h"
#include "ui_imageinfopanel.h"

ImageInfoPanel::ImageInfoPanel(QWidget* parent)
    : QFrame(parent)
    , ui(new Ui::Frame)
{
    ui->setupUi(this);
}
ImageInfoPanel::~ImageInfoPanel()
{
}


void ImageInfoPanel::SetCompressionText(const QString& text) {
    ui->lblImgPropCompression->setText(text);
}

void ImageInfoPanel::SetWidthText(const QString& text) {
    ui->lblImgPropWidth->setText(text);
}

void ImageInfoPanel::SetHeightText(const QString& text) {
    ui->lblImgPropHeight->setText(text);
}

void ImageInfoPanel::SetMipsText(const QString& text) {
    ui->lblImgPropMips->setText(text);
}
