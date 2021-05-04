#include "SimpleRibbonVBar.h"

SimpleRibbonVBar::SimpleRibbonVBar(QWidget* parent)
    : QWidget(parent)
    , mLayout(new QVBoxLayout())
{
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    gridLayout->addLayout(mLayout, 0, 0, 1, 1);

    //#NOTE_SK: this "spacer" widget will act as a filler
    QWidget* spacerWidget = new QWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(spacerWidget->sizePolicy().hasHeightForWidth());
    spacerWidget->setSizePolicy(sizePolicy);

    gridLayout->addWidget(spacerWidget, 1, 0, 1, 1);
}

SimpleRibbonVBar::~SimpleRibbonVBar() {

}


void SimpleRibbonVBar::AddWidget(QWidget* widget) {
    mLayout->addWidget(widget);
}
