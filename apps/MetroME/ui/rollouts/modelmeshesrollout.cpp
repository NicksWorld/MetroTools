#include "modelmeshesrollout.h"
#include "ui_modelmeshesrollout.h"

#include "../props/objectpropertybrowser.h"
#include "../props/materialstringsprop.h"

#include "metro/MetroModel.h"


ModelMeshesRollout::ModelMeshesRollout(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModelMeshesRollout)
    , mModel(nullptr)
    , mSelectedGD(-1)
{
    ui->setupUi(this);

    connect(ui->modelPropertyBrowser, &ObjectPropertyBrowser::objectPropertyChanged, this, &ModelMeshesRollout::OnPropertyBrowserObjectPropertyChanged);
}

ModelMeshesRollout::~ModelMeshesRollout() {
    delete ui;
}


void ModelMeshesRollout::FillForTheModel(MetroModelBase* model) {
    mModel = model;
    mSelectedGD = -1;

    ui->treeModelHierarchy->clear();

    if (mModel) {
        MyArray<MetroModelGeomData> gds;
        model->CollectGeomData(gds);

        if (!gds.empty()) {
            QTreeWidgetItem* topNode = new QTreeWidgetItem({ QLatin1String("Model") });
            topNode->setData(0, Qt::UserRole, QVariant(int(-1)));
            ui->treeModelHierarchy->addTopLevelItem(topNode);

            int idx = 0;
            for (const auto& gd : gds) {
                QString childText = QString("Mesh_%1").arg(idx);
                QTreeWidgetItem* child = new QTreeWidgetItem({ childText });
                child->setData(0, Qt::UserRole, QVariant(idx));

                topNode->addChild(child);

                ++idx;
            }

            ui->treeModelHierarchy->expandAll();
        }
    }

    ui->modelPropertyBrowser->setActiveObject(nullptr);
    mMatStringsProp = nullptr;
}

int ModelMeshesRollout::GetSelectedMeshIdx() const {
    return mSelectedGD;
}



void ModelMeshesRollout::on_treeModelHierarchy_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    const int oldSelectedGD = mSelectedGD;

    QTreeWidgetItem* item = ui->treeModelHierarchy->currentItem();
    if (item) {
        const int gdIdx = item->data(0, Qt::UserRole).toInt();

        if (gdIdx >= 0 && mModel) {
            MyArray<MetroModelGeomData> gds;
            mModel->CollectGeomData(gds);

            const MetroModelBase* gdModel = gds[gdIdx].model;

            ui->modelPropertyBrowser->setActiveObject(nullptr);

            mMatStringsProp = MakeStrongPtr<MaterialStringsProp>();
            mMatStringsProp->texture = QString::fromStdString(gdModel->GetMaterialString(0));
            mMatStringsProp->shader = QString::fromStdString(gdModel->GetMaterialString(1));
            mMatStringsProp->material = QString::fromStdString(gdModel->GetMaterialString(2));
            mMatStringsProp->src_mat = QString::fromStdString(gdModel->GetMaterialString(3));

            ui->modelPropertyBrowser->setActiveObject(mMatStringsProp.get());

            mSelectedGD = gdIdx;
        }
    } else {
        mSelectedGD = -1;
        ui->modelPropertyBrowser->setActiveObject(nullptr);
        mMatStringsProp = nullptr;
    }

    if (oldSelectedGD != mSelectedGD) {
        emit SignalMeshSelectionChanged(mSelectedGD);
    }
}

void ModelMeshesRollout::OnPropertyBrowserObjectPropertyChanged() {
    if (mSelectedGD >= 0) {
        if (mModel) {
            MyArray<MetroModelGeomData> gds;
            mModel->CollectGeomData(gds);

            MetroModelBase* gdModel = const_cast<MetroModelBase*>(gds[mSelectedGD].model);

            bool updateModel = false;

            if (mMatStringsProp->texture.toStdString() != gdModel->GetMaterialString(0)) {
                gdModel->SetMaterialString(mMatStringsProp->texture.toStdString(), 0);
                updateModel = true;
            }
            if (mMatStringsProp->shader.toStdString() != gdModel->GetMaterialString(1)) {
                gdModel->SetMaterialString(mMatStringsProp->shader.toStdString(), 1);
                updateModel = true;
            }
            if (mMatStringsProp->material.toStdString() != gdModel->GetMaterialString(2)) {
                gdModel->SetMaterialString(mMatStringsProp->material.toStdString(), 2);
                updateModel = true;
            }
            if (mMatStringsProp->src_mat.toStdString() != gdModel->GetMaterialString(3)) {
                gdModel->SetMaterialString(mMatStringsProp->src_mat.toStdString(), 3);
                updateModel = true;
            }

            if (updateModel) {
                emit SignalMeshPropertiesChanged();
            }
        }
    }
}
