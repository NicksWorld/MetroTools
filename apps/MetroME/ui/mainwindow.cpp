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

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include "importers/ImporterOBJ.h"
#include "exporters/ExporterOBJ.h"
#include "exporters/ExporterFBX.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mRenderPanel(nullptr)
    , mPropertyBrowser(new ObjectPropertyBrowser(this))
    , mSelectedGD(-1)
    , mMatStringsProp{}
{
    ui->setupUi(this);

    connect(ui->ribbon, &MainRibbon::SignalFileImportMetroModel, this, &MainWindow::OnImportMetroModel);
    connect(ui->ribbon, &MainRibbon::SignalFileImportOBJModel, this, &MainWindow::OnImportOBJModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportMetroModel, this, &MainWindow::OnExportMetroModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportOBJModel, this, &MainWindow::OnExportOBJModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportFBXModel, this, &MainWindow::OnExportFBXModel);
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

    mPropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->pnlProperties->layout()->addWidget(mPropertyBrowser);
    connect(mPropertyBrowser, &ObjectPropertyBrowser::objectPropertyChanged, this, &MainWindow::OnPropertyBrowserObjectPropertyChanged);

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

    const MetroModelType mtype = model->GetModelType();
    RefPtr<MetroSkeleton> skeleton;

    if (mtype == MetroModelType::Skeleton ||
        mtype == MetroModelType::Skeleton2 ||
        mtype == MetroModelType::Skeleton3) {
        skeleton = scast<MetroModelSkeleton*>(model)->GetSkeleton();
    }

    ui->ribbon->EnableSkeletonTab(skeleton != nullptr);

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);

    if (!gds.empty()) {
        ui->treeModelHierarchy->clear();
        QTreeWidgetItem* top = new QTreeWidgetItem({ QLatin1String("Model") });
        top->setData(0, Qt::UserRole, QVariant(int(-1)));
        ui->treeModelHierarchy->addTopLevelItem(top);

        int idx = 0;
        for (const auto& gd : gds) {
            QString childText = QString("Mesh_%1").arg(idx);
            QTreeWidgetItem* child = new QTreeWidgetItem({ childText });
            child->setData(0, Qt::UserRole, QVariant(idx));

            top->addChild(child);

            ++idx;
        }

        ui->treeModelHierarchy->expandAll();

        mPropertyBrowser->setActiveObject(nullptr);
    }
}


void MainWindow::OnWindowLoaded() {
    QSize mainSize = ui->frame->size();
    const int panelSize = mainSize.width() / 4;
    const int renderSize = mainSize.width() - panelSize;

    QList<int> widgetSizes;
    widgetSizes << renderSize << panelSize;
    ui->splitterMain->setSizes(widgetSizes);
}

void MainWindow::OnImportMetroModel() {
    QString name = QFileDialog::getOpenFileName(this, tr("Choose Metro model/mesh file..."), QString(), tr("Metro model files (*.model);;Metro mesh files (*.mesh);;All files (*.*)"));
    if (name.length() > 3) {
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
    if (name.length() > 3) {
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
    if (name.length() > 3) {
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
    if (name.length() > 3) {
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
    if (name.length() > 3) {
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

void MainWindow::on_treeModelHierarchy_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
    QTreeWidgetItem* item = ui->treeModelHierarchy->currentItem();
    if (item) {
        const int gdIdx = item->data(0, Qt::UserRole).toInt();

        if (gdIdx >= 0 && mRenderPanel) {
            RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
            if (model) {
                MyArray<MetroModelGeomData> gds;
                model->CollectGeomData(gds);

                const MetroModelBase* gdModel = gds[gdIdx].model;

                mPropertyBrowser->setActiveObject(nullptr);

                mMatStringsProp = MakeStrongPtr<MaterialStringsProp>();
                mMatStringsProp->texture = QString::fromStdString(gdModel->GetMaterialString(0));
                mMatStringsProp->shader = QString::fromStdString(gdModel->GetMaterialString(1));
                mMatStringsProp->material = QString::fromStdString(gdModel->GetMaterialString(2));
                mMatStringsProp->src_mat = QString::fromStdString(gdModel->GetMaterialString(3));

                mPropertyBrowser->setActiveObject(mMatStringsProp.get());

                mSelectedGD = gdIdx;
            }
        }
    } else {
        mSelectedGD = -1;
        mPropertyBrowser->setActiveObject(nullptr);
    }
}
