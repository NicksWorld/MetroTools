#include "SimpleRibbonTab.h"
#include "SimpleRibbonGroup.h"

SimpleRibbonTab::SimpleRibbonTab(QWidget* parent)
    : QWidget(parent)
    , mGridLayout(nullptr)
    , mHorizontalLayout(nullptr)
{
    mGridLayout = new QGridLayout(this);
    mGridLayout->setSpacing(0);
    mGridLayout->setContentsMargins(0, 0, 0, 0);

    mHorizontalLayout = new QHBoxLayout();
    mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
    mHorizontalLayout->setSpacing(0);

    mGridLayout->addLayout(mHorizontalLayout, 0, 0, 1, 1);

    //#NOTE_SK: this "spacer" widget will act as a filler to hold the actual content from stretching all over
    QWidget* spacerWidget = new QWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(spacerWidget->sizePolicy().hasHeightForWidth());
    spacerWidget->setSizePolicy(sizePolicy);

    mGridLayout->addWidget(spacerWidget, 0, 1, 1, 1);
}

SimpleRibbonTab::~SimpleRibbonTab() {

}


QString SimpleRibbonTab::GetTabName() const {
    return QObject::objectName();
}

SimpleRibbonGroup* SimpleRibbonTab::AddRibbonGroup(const QString& title) {
    SimpleRibbonGroup* newGroup = new SimpleRibbonGroup();
    newGroup->SetTitle(title);

    mHorizontalLayout->addWidget(newGroup);

    return newGroup;
}

SimpleRibbonGroup* SimpleRibbonTab::FindRibbonGroup(const QString& title) {
    SimpleRibbonGroup* result = nullptr;

    QString lowerTitle = title.toLower();
    for (int i = 0, end = mHorizontalLayout->count(); i < end; ++i) {
        SimpleRibbonGroup* group = static_cast<SimpleRibbonGroup*>(mHorizontalLayout->itemAt(i)->widget());
        if (group->GetTitle().toLower() == lowerTitle) {
            result = group;
            break;
        }
    }

    return result;
}

void SimpleRibbonTab::RemoveRibbonGroup(const QString& title) {
    SimpleRibbonGroup* group = this->FindRibbonGroup(title);
    if (group) {
        mHorizontalLayout->removeWidget(group);
    }
}
