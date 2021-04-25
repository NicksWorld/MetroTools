#include "SimpleRibbonGroup.h"

SimpleRibbonGroup::SimpleRibbonGroup(QWidget* parent)
    : QWidget(parent)
    , mGridLayout(nullptr)
    , mVerticalLayout(nullptr)
    , mHorizontalLayout(nullptr)
    , mTitleLabel(nullptr)
    , mVerticalLine(nullptr)
{
    this->SetupLayout();
}

SimpleRibbonGroup::~SimpleRibbonGroup() {

}

void SimpleRibbonGroup::SetTitle(const QString& title) {
    mTitleLabel->setText(title);
}

QString SimpleRibbonGroup::GetTitle() const {
    return mTitleLabel->text();
}

void SimpleRibbonGroup::AddWidget(QWidget* widget) {
    mHorizontalLayout->addWidget(widget);
}



void SimpleRibbonGroup::SetupLayout() {
    mGridLayout = new QGridLayout(this);
    mGridLayout->setContentsMargins(0, 0, 0, 0);
    mGridLayout->setSpacing(0);

    mVerticalLayout = new QVBoxLayout();
    mVerticalLayout->setContentsMargins(1, 1, 1, 1);
    mVerticalLayout->setSpacing(0);

    mHorizontalLayout = new QHBoxLayout();
    mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
    mHorizontalLayout->setSpacing(0);

    mVerticalLayout->addLayout(mHorizontalLayout);

    mTitleLabel = new QLabel(this);
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mTitleLabel->setStyleSheet(QString::fromUtf8("font-weight: normal; font-style: italic; color: #c0c0c0;"));

    mVerticalLayout->addWidget(mTitleLabel);

    mGridLayout->addLayout(mVerticalLayout, 0, 0, 1, 1);

    mVerticalLine = new QFrame(this);
    mVerticalLine->setStyleSheet(QString::fromUtf8("color: #c0c0c0;"));
    mVerticalLine->setFrameShadow(QFrame::Plain);
    mVerticalLine->setFrameShape(QFrame::VLine);

    mGridLayout->addWidget(mVerticalLine, 0, 1, 1, 1);
}
