#include "boneslistrollout.h"
#include "ui_boneslistrollout.h"

#include "metro/MetroSkeleton.h"

BonesListRollout::BonesListRollout(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BonesListRollout)
{
    ui->setupUi(this);

    ui->bonesTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

BonesListRollout::~BonesListRollout() {
    delete ui;
}

void BonesListRollout::FillForTheSkeleton(const MetroSkeleton* skeleton) {
    ui->bonesTree->clear();

    if (skeleton) {
        QTreeWidgetItem* topBonesNode = new QTreeWidgetItem({ QLatin1String("Bones") });
        topBonesNode->setData(0, Qt::UserRole, QVariant(int(-1)));
        ui->bonesTree->addTopLevelItem(topBonesNode);

        QTreeWidgetItem* topLocatorsNode = new QTreeWidgetItem({ QLatin1String("Locators") });
        topLocatorsNode->setData(0, Qt::UserRole, QVariant(int(-1)));
        ui->bonesTree->addTopLevelItem(topLocatorsNode);

        const size_t numBones = skeleton->GetNumBones();
        MyArray<QTreeWidgetItem*> boneNodes(numBones);
        // 1st pass - create all nodes
        for (size_t i = 0; i < numBones; ++i) {
            QString boneName = QString::fromStdString(skeleton->GetBoneName(i));
            boneNodes[i] = new QTreeWidgetItem({ boneName });
            boneNodes[i]->setData(0, Qt::UserRole, QVariant(scast<int>(i)));
        }
        // 2nd pass - make the hierarchy
        for (size_t i = 0; i < numBones; ++i) {
            const size_t parentIdx = skeleton->GetBoneParentIdx(i);
            if (kInvalidValue == parentIdx) {
                topBonesNode->addChild(boneNodes[i]);
            } else {
                boneNodes[parentIdx]->addChild(boneNodes[i]);
            }
        }

        const size_t numLocators = skeleton->GetNumLocators();
        for (size_t i = 0; i < numLocators; ++i) {
            const size_t fulIdx = i + numBones;
            QString locatorName = QString::fromStdString(skeleton->GetBoneName(fulIdx));
            QTreeWidgetItem* locatorNode = new QTreeWidgetItem({ locatorName });
            locatorNode->setData(0, Qt::UserRole, QVariant(scast<int>(fulIdx)));
            topLocatorsNode->addChild(locatorNode);
        }
    }
}

void BonesListRollout::on_bonesTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {

}
