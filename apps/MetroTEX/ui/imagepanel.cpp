#include "imagepanel.h"

#include <QImage>
#include <QMouseEvent>
#include <QScrollBar>
#include <QApplication>

#include "mymath.h"


MyLabel::MyLabel(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f) {}

void MyLabel::mouseMoveEvent(QMouseEvent* ev) {
    ev->ignore();
}

void MyLabel::mousePressEvent(QMouseEvent* ev) {
    ev->ignore();
}

void MyLabel::mouseReleaseEvent(QMouseEvent* ev) {
    ev->ignore();
}



ImagePanel::ImagePanel(QWidget* parent)
    : QScrollArea(parent)
    , mImageLabel(nullptr)
    , mTransparency(false)
    , mImageWidth(0)
    , mImageHeight(0)
    , mLMBDown(false)
    , mLastMPos(0, 0)
{
    mImageLabel = new MyLabel();
    mImageLabel->setBackgroundRole(QPalette::Base);
    mImageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageLabel->setScaledContents(true);

    QPalette palette;
    QImage img(16, 16, QImage::Format_RGB888);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const bool isWhite = (x < 8 && y < 8) || (x >= 8 && y >= 8);
            img.setPixelColor(x, y, isWhite ? QColor(0xD3, 0xD3, 0xD3) : QColor(0xA9, 0xA9, 0xA9));
        }
    }
    palette.setBrush(mImageLabel->backgroundRole(), QBrush(img));
    mImageLabel->setPalette(palette);

    this->setCursor(Qt::OpenHandCursor);

    this->setBackgroundRole(QPalette::Base);
    this->setWidget(mImageLabel);
}

void ImagePanel::SetImage(const void* pixelsRGBA, const size_t width, const size_t height) {
    if (pixelsRGBA) {
        mImageData.resize(width * height * 4);
        memcpy(mImageData.data(), pixelsRGBA, mImageData.size());

        mImageWidth = scast<int>(width);
        mImageHeight = scast<int>(height);
    } else {
        mImageData.clear();
        mImageWidth = 0;
        mImageHeight = 0;
    }

    this->ShowTransparency(mTransparency);
    this->ResetScroll();
}

const void* ImagePanel::GetImageData() const {
    return mImageData.empty() ? nullptr : mImageData.data();
}

size_t ImagePanel::GetImageWidth() const {
    return mImageWidth;
}

size_t ImagePanel::GetImageHeight() const {
    return mImageHeight;
}

void ImagePanel::ShowTransparency(const bool toShow) {
    if (!mImageData.empty()) {
        QImage newImage(rcast<const uchar*>(mImageData.data()), mImageWidth, mImageHeight, toShow ? QImage::Format_RGBA8888 : QImage::Format_RGBX8888);
        mImageLabel->setPixmap(QPixmap::fromImage(newImage));
        mImageLabel->resize(mImageWidth, mImageHeight);
    } else {
        mImageLabel->resize(1, 1);
        mImageLabel->setPixmap(QPixmap());
    }

    mTransparency = toShow;
}

// my own events
void ImagePanel::mouseMoveEvent(QMouseEvent* ev) {
    if (mLMBDown) {
        QPoint pt = ev->pos();
        const int dx = mLastMPos.x() - pt.x();
        const int dy = mLastMPos.y() - pt.y();
        mLastMPos = pt;

        if (dx) {
            QScrollBar* sbar = this->horizontalScrollBar();
            if (sbar) {
                sbar->setSliderPosition(sbar->sliderPosition() + dx);
            }
        }

        if (dy) {
            QScrollBar* sbar = this->verticalScrollBar();
            if (sbar) {
                sbar->setSliderPosition(sbar->sliderPosition() + dy);
            }
        }
    }

    ev->accept();
}

void ImagePanel::mousePressEvent(QMouseEvent* ev) {
    if (Qt::LeftButton == ev->button()) {
        mLMBDown = true;
        mLastMPos = ev->pos();

        this->unsetCursor();
        this->setCursor(Qt::ClosedHandCursor);
        this->update();
    }

    ev->accept();
}

void ImagePanel::mouseReleaseEvent(QMouseEvent* ev) {
    if (Qt::LeftButton == ev->button()) {
        mLMBDown = false;

        this->unsetCursor();
        this->setCursor(Qt::OpenHandCursor);
        this->update();
    }

    ev->accept();
}

void ImagePanel::ResetScroll() {
    QScrollBar* hbar = this->horizontalScrollBar();
    if (hbar) {
        hbar->setValue(hbar->minimum());
    }

    QScrollBar* vbar = this->verticalScrollBar();
    if (vbar) {
        vbar->setValue(vbar->minimum());
    }
}