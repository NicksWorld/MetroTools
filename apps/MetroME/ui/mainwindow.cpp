#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderpanel.h"

#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>

#include "props/materialstringsprop.h"

#include "metro/MetroContext.h"
#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include "importers/ImporterOBJ.h"
#include "exporters/ExporterOBJ.h"
#include "exporters/ExporterFBX.h"
#include "exporters/ExporterGLTF.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mRenderPanel(nullptr)
    , mModelHierarchyTree(new QTreeWidget)
    , mSkeletonHierarchyTree(new QTreeWidget)
    , mModelPropertyBrowser(new ObjectPropertyBrowser)
    , mSkeletonPropertyBrowser(new ObjectPropertyBrowser)
    , mSelectedGD(-1)
    , mMatStringsProp{}
    , mIsInSkeletonView(false)
{
    ui->setupUi(this);

    // ribbon
    connect(ui->ribbon, &MainRibbon::SignalCurrentTabChanged, this, &MainWindow::OnRibbonTabChanged);
    //
    connect(ui->ribbon, &MainRibbon::SignalFileImportMetroModel, this, &MainWindow::OnImportMetroModel);
    connect(ui->ribbon, &MainRibbon::SignalFileImportOBJModel, this, &MainWindow::OnImportOBJModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportMetroModel, this, &MainWindow::OnExportMetroModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportOBJModel, this, &MainWindow::OnExportOBJModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportFBXModel, this, &MainWindow::OnExportFBXModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportGLTFModel, this, &MainWindow::OnExportGLTFModel);
    //
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBoundsChecked, this, &MainWindow::OnShowBounds);
    connect(ui->ribbon, &MainRibbon::Signal3DViewBoundsTypeChanged, this, &MainWindow::OnBoundsTypeChanged);
    connect(ui->ribbon, &MainRibbon::Signal3DViewSubmodelsBoundsChecked, this, &MainWindow::OnSubmodelBounds);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesChecked, this, &MainWindow::OnSkeletonShowBones);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesLinksChecked, this, &MainWindow::OnSkeletonShowBonesLinks);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesNamesChecked, this, &MainWindow::OnSkeletonShowBonesNames);

    mRenderPanel = new RenderPanel(ui->renderContainer);
    mRenderPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRenderPanel->setGeometry(QRect(0, 0, ui->renderContainer->width(), ui->renderContainer->height()));
    ui->renderContainer->layout()->addWidget(mRenderPanel);

    // trees
    mModelHierarchyTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelHierarchyTree->setHeaderHidden(true);
    mSkeletonHierarchyTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSkeletonHierarchyTree->setHeaderHidden(true);
    ui->pnlTreeView->layout()->addWidget(mModelHierarchyTree);
    ui->pnlTreeView->layout()->addWidget(mSkeletonHierarchyTree);
    mModelHierarchyTree->show();
    mSkeletonHierarchyTree->hide();
    connect(mModelHierarchyTree, &QTreeWidget::currentItemChanged, this, &MainWindow::OnModelHierarchyTreeCurrentItemChanged);
    connect(mSkeletonHierarchyTree, &QTreeWidget::currentItemChanged, this, &MainWindow::OnSkeletonHierarchyTreeCurrentItemChanged);

    // property views
    mModelPropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSkeletonPropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->pnlProperties->layout()->addWidget(mModelPropertyBrowser);
    ui->pnlProperties->layout()->addWidget(mSkeletonPropertyBrowser);
    mModelPropertyBrowser->show();
    mSkeletonPropertyBrowser->hide();
    connect(mModelPropertyBrowser, &ObjectPropertyBrowser::objectPropertyChanged, this, &MainWindow::OnPropertyBrowserObjectPropertyChanged);

    // renderer
    bool deviceIsOk = u4a::Renderer::Get().CreateDevice(u4a::Renderer::IF_D2D_Support);
    if (!deviceIsOk) {
        QMessageBox::critical(this, this->windowTitle(), tr("Failed to create DirectX 11 graphics!\n3D viewer will be unavailable."));
    } else {
        if (!u4a::Renderer::Get().Initialize()) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to finish DirectX 11 renderer initialization!\n3D viewer will be unavailable."));
        } else {
            u4a::ResourcesManager::Get().Initialize();
            u4a::ResourcesManager::Get().SetLoadHighRes(true);
        }

        if (!mRenderPanel->Initialize()) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to initialize Render panel!\n3D viewer will be unavailable."));
        }
    }

    MetroContext::Get().InitFromContentFolder(R"(e:\Games\SteamLibrary\steamapps\common\Metro Last Light Redux\!!\extracted\content)");

    QTimer::singleShot(0, this, SLOT(OnWindowLoaded()));
}

MainWindow::~MainWindow() {
    delete ui;
}



void MainWindow::UpdateUIForTheModel(MetroModelBase* model) {
    mSelectedGD = -1;

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);

    if (!gds.empty()) {
        mModelHierarchyTree->clear();
        QTreeWidgetItem* topNode = new QTreeWidgetItem({ QLatin1String("Model") });
        topNode->setData(0, Qt::UserRole, QVariant(int(-1)));
        mModelHierarchyTree->addTopLevelItem(topNode);

        int idx = 0;
        for (const auto& gd : gds) {
            QString childText = QString("Mesh_%1").arg(idx);
            QTreeWidgetItem* child = new QTreeWidgetItem({ childText });
            child->setData(0, Qt::UserRole, QVariant(idx));

            topNode->addChild(child);

            ++idx;
        }

        mModelHierarchyTree->expandAll();

        mModelPropertyBrowser->setActiveObject(nullptr);
    }

    RefPtr<MetroSkeleton> skeleton = model->IsSkeleton() ? scast<MetroModelSkeleton*>(model)->GetSkeleton() : nullptr;
    if (skeleton) {
        ui->ribbon->EnableTab(MainRibbon::TabType::Skeleton, true);

        mSkeletonHierarchyTree->clear();
        QTreeWidgetItem* topBonesNode = new QTreeWidgetItem({ QLatin1String("Bones") });
        topBonesNode->setData(0, Qt::UserRole, QVariant(int(-1)));
        mSkeletonHierarchyTree->addTopLevelItem(topBonesNode);

        QTreeWidgetItem* topLocatorsNode = new QTreeWidgetItem({ QLatin1String("Locators") });
        topLocatorsNode->setData(0, Qt::UserRole, QVariant(int(-1)));
        mSkeletonHierarchyTree->addTopLevelItem(topLocatorsNode);

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


void MainWindow::OnWindowLoaded() {
    QSize mainSize = ui->frame->size();
    const int panelSize = mainSize.width() / 4;
    const int renderSize = mainSize.width() - panelSize;

    QList<int> widgetSizes;
    widgetSizes << renderSize << panelSize;
    ui->splitterMain->setSizes(widgetSizes);

    ui->ribbon->EnableTab(MainRibbon::TabType::Skeleton, false);
    ui->ribbon->EnableTab(MainRibbon::TabType::Animation, false);
}

void MainWindow::OnRibbonTabChanged(const MainRibbon::TabType tab) {
    if (MainRibbon::TabType::Model == tab) {
        mModelHierarchyTree->show();
        mModelPropertyBrowser->show();
        mSkeletonHierarchyTree->hide();
        mSkeletonPropertyBrowser->hide();
    } else if (MainRibbon::TabType::Skeleton == tab) {
        mModelHierarchyTree->hide();
        mModelPropertyBrowser->hide();
        mSkeletonHierarchyTree->show();
        mSkeletonPropertyBrowser->show();

        this->OnSkeletonShowBones(true);
        this->OnSkeletonShowBonesLinks(true);
    }
}

void MainWindow::OnImportMetroModel() {
    QString name = QFileDialog::getOpenFileName(this, tr("Choose Metro model/mesh file..."), QString(), tr("Metro model files (*.model);;Metro mesh files (*.mesh);;All files (*.*)"));
    if (!name.isEmpty()) {
        fs::path fullPath = name.toStdWString();

        MemStream stream = OSReadFile(fullPath);
        if (stream) {
            MetroModelLoadParams params = {
                kEmptyString,
                kEmptyString,
                0,
                MetroModelLoadParams::LoadEverything,
                MetroFSPath(fullPath)
            };

            RefPtr<MetroModelBase> model = MetroModelFactory::CreateModelFromStream(stream, params);
            if (model && mRenderPanel) {
                mRenderPanel->SetModel(model);

                this->UpdateUIForTheModel(model.get());
            }
        }
    }
}

void MainWindow::OnImportOBJModel() {
    QString name = QFileDialog::getOpenFileName(this, tr("Choose OBJ model file..."), QString(), tr("OBJ model files (*.obj);;All files (*.*)"));
    if (!name.isEmpty()) {
        fs::path fullPath = name.toStdWString();

        ImporterOBJ importer;
        RefPtr<MetroModelBase> model = importer.ImportModel(fullPath);
        if (model && mRenderPanel) {
            mRenderPanel->SetModel(model);

            this->UpdateUIForTheModel(model.get());
        }
    }
}

void MainWindow::OnExportMetroModel() {
    QString name = QFileDialog::getSaveFileName(this, tr("Where to export Metro model..."), QString(), tr("Metro Model file (*.model);;All files (*.*)"));
    if (!name.isEmpty()) {
        RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
        if (model) {
            MemWriteStream stream;
            if (model->Save(stream)) {
                fs::path fullPath = name.toStdWString();
                OSWriteFile(fullPath, stream.Data(), stream.GetWrittenBytesCount());
            }
        }
    }
}

void MainWindow::OnExportOBJModel() {
    QString name = QFileDialog::getSaveFileName(this, tr("Where to export OBJ model..."), QString(), tr("OBJ model file (*.obj);;All files (*.*)"));
    if (!name.isEmpty()) {
        RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
        if (model) {
            fs::path fullPath = name.toStdWString();

            ExporterOBJ expObj;
            expObj.SetExcludeCollision(true);
            expObj.SetExporterName("MetroME");
            expObj.SetTexturesExtension(".tga");
            expObj.ExportModel(*model, fullPath);
        }
    }
}

void MainWindow::OnExportFBXModel() {
    QString name = QFileDialog::getSaveFileName(this, tr("Where to export FBX model..."), QString(), tr("FBX model file (*.fbx);;All files (*.*)"));
    if (!name.isEmpty()) {
        RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
        if (model) {
            fs::path fullPath = name.toStdWString();

            ExporterFBX expFbx;
            expFbx.SetExportMesh(true);
            expFbx.SetExportSkeleton(true);
            expFbx.SetExcludeCollision(true);
            expFbx.SetExportAnimation(true);

            expFbx.SetExporterName("MetroME");
            expFbx.SetTexturesExtension(".tga");
            expFbx.ExportModel(*model, fullPath);
        }
    }
}

void MainWindow::OnExportGLTFModel() {
    QString name = QFileDialog::getSaveFileName(this, tr("Where to export GLTF model..."), QString(), tr("FBX model file (*.gltf);;All files (*.*)"));
    if (!name.isEmpty()) {
        RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
        if (model) {
            fs::path fullPath = name.toStdWString();

            ExporterGLTF expGltf;
            expGltf.SetExportMesh(true);
            expGltf.SetExportSkeleton(true);
            expGltf.SetExcludeCollision(true);
            expGltf.SetExportAnimation(true);

            expGltf.SetExporterName("MetroME");
            expGltf.SetTexturesExtension(".tga");
            expGltf.ExportModel(*model, fullPath);
        }
    }
}

//
void MainWindow::OnShowBounds(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugShowBounds(checked);
    }
}

void MainWindow::OnBoundsTypeChanged(int index) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugBoundsType(scast<RenderPanel::DebugBoundsType>(index));
    }
}

void MainWindow::OnSubmodelBounds(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugShowSubmodelsBounds(checked);
    }
}

void MainWindow::OnSkeletonShowBones(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBones(checked);
    }
}

void MainWindow::OnSkeletonShowBonesLinks(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBonesLinks(checked);
    }
}

void MainWindow::OnSkeletonShowBonesNames(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBonesNames(checked);
    }
}

void MainWindow::OnPropertyBrowserObjectPropertyChanged() {
    if (mSelectedGD >= 0 && mRenderPanel) {
        RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
        if (model) {
            MyArray<MetroModelGeomData> gds;
            model->CollectGeomData(gds);

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
                mRenderPanel->UpdateModelProps();
            }
        }
    }
}

void MainWindow::OnModelHierarchyTreeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
    QTreeWidgetItem* item = mModelHierarchyTree->currentItem();
    if (item) {
        const int gdIdx = item->data(0, Qt::UserRole).toInt();

        if (gdIdx >= 0 && mRenderPanel) {
            RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
            if (model) {
                MyArray<MetroModelGeomData> gds;
                model->CollectGeomData(gds);

                const MetroModelBase* gdModel = gds[gdIdx].model;

                mModelPropertyBrowser->setActiveObject(nullptr);

                mMatStringsProp = MakeStrongPtr<MaterialStringsProp>();
                mMatStringsProp->texture = QString::fromStdString(gdModel->GetMaterialString(0));
                mMatStringsProp->shader = QString::fromStdString(gdModel->GetMaterialString(1));
                mMatStringsProp->material = QString::fromStdString(gdModel->GetMaterialString(2));
                mMatStringsProp->src_mat = QString::fromStdString(gdModel->GetMaterialString(3));

                mModelPropertyBrowser->setActiveObject(mMatStringsProp.get());

                mSelectedGD = gdIdx;
            }
        }
    } else {
        mSelectedGD = -1;
        mModelPropertyBrowser->setActiveObject(nullptr);
    }
}

void MainWindow::OnSkeletonHierarchyTreeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {

}
