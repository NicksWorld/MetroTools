#include "SimpleRibbonVBar.h"

SimpleRibbonVBar::SimpleRibbonVBar(QWidget* parent)
    : QWidget(parent)
    , mLayout(new QVBoxLayout()) {
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    this->setLayout(mLayout);
}

SimpleRibbonVBar::~SimpleRibbonVBar() {

}


void SimpleRibbonVBar::AddWidget(QWidget* widget) {
    mLayout->addWidget(widget);
}
