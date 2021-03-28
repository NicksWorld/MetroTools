#pragma once
#include <QScrollArea>
#include <QLabel>

#include "mycommon.h"

class MyLabel : public QLabel {
    Q_OBJECT

public:
    explicit MyLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
};

class ImagePanel : public QScrollArea {
    Q_OBJECT

public:
    ImagePanel(QWidget* parent = nullptr);

    void        SetImage(const void* pixelsRGBA, const size_t width, const size_t height);
    void        ShowTransparency(const bool toShow);

protected:
    void        mouseMoveEvent(QMouseEvent* ev) override;
    void        mousePressEvent(QMouseEvent* ev) override;
    void        mouseReleaseEvent(QMouseEvent* ev) override;

private:
    void        ResetScroll();

private:
    QLabel*     mImageLabel;
    bool        mTransparency;
    BytesArray  mImageData;
    int         mImageWidth;
    int         mImageHeight;
    bool        mLMBDown;
    QPoint      mLastMPos;
};
