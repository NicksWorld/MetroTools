#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderpanel.h"
#include "sessionsdlg.h"
#include "exportmodeldlg.h"

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
#include "importers/ImporterFBX.h"
#include "exporters/ExporterOBJ.h"
#include "exporters/ExporterFBX.h"
#include "exporters/ExporterGLTF.h"

#include "../MetroSessions.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mRenderPanel(nullptr)
    , mModelHierarchyTree(new QTreeWidget)
    , mModelPropertyBrowser(new ObjectPropertyBrowser)
    , mBonesListRollout(new BonesListRollout)
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
    connect(ui->ribbon, &MainRibbon::SignalFileImportFBXModel, this, &MainWindow::OnImportFBXModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportMetroModel, this, &MainWindow::OnExportMetroModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportOBJModel, this, &MainWindow::OnExportOBJModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportFBXModel, this, &MainWindow::OnExportFBXModel);
    connect(ui->ribbon, &MainRibbon::SignalFileExportGLTFModel, this, &MainWindow::OnExportGLTFModel);
    //
    connect(ui->ribbon, &MainRibbon::SignalFileImportMetroSkeleton, this, &MainWindow::OnImportMetroSkeleton);
    connect(ui->ribbon, &MainRibbon::SignalFileImportFBXSkeleton, this, &MainWindow::OnImportFBXSkeleton);
    connect(ui->ribbon, &MainRibbon::SignalFileExportMetroSkeleton, this, &MainWindow::OnExportMetroSkeleton);
    connect(ui->ribbon, &MainRibbon::SignalFileExportFBXSkeleton, this, &MainWindow::OnExportFBXSkeleton);
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

    ui->toolbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolbox->addWidget(mBonesListRollout);
    ui->toolbox->hide();

    // trees
    mModelHierarchyTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelHierarchyTree->setHeaderHidden(true);
    ui->pnlTreeView->layout()->addWidget(mModelHierarchyTree);
    mModelHierarchyTree->show();
    connect(mModelHierarchyTree, &QTreeWidget::currentItemChanged, this, &MainWindow::OnModelHierarchyTreeCurrentItemChanged);

    // property views
    mModelPropertyBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->pnlProperties->layout()->addWidget(mModelPropertyBrowser);
    mModelPropertyBrowser->show();
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
        mBonesListRollout->FillForTheSkeleton(skeleton.get());
    }
}


void MainWindow::OnWindowLoaded() {
    QSize mainSize = ui->frame->size();
    const int panelSize = mainSize.width() / 4;
    const int renderSize = mainSize.width() - panelSize;

    QList<int> widgetSizes;
    widgetSizes << renderSize << panelSize;
    ui->splitterMain->setSizes(widgetSizes);

    fs::path sessionsPath = fs::path(qApp->applicationDirPath().toStdWString()) / "sessions.met";

    MetroSessionsList sessionsList;
    sessionsList.LoadFromFile(sessionsPath);

    SessionsDlg sessionsDlg(this);
    sessionsDlg.SetSessionsList(&sessionsList);
    if (QDialog::Accepted == sessionsDlg.exec()) {
        if (sessionsDlg.IsUseExistingSession()) {
            const MetroSession& session = sessionsList.GetSession(sessionsDlg.GetExistingSessionIdx());

            MetroContext::Get().InitFromContentFolder(session.GetGameVersion(), session.GetContentFolder());
        } else {
            MetroSession newSession;
            newSession.SetGameVersion(scast<MetroGameVersion>(sessionsDlg.GetNewSessionGameVersion()));
            newSession.SetContentFolder(sessionsDlg.GetNewSessionContentFolder());
            sessionsList.AddSession(newSession);

            sessionsList.SaveToFile(sessionsPath);

            MetroContext::Get().InitFromContentFolder(newSession.GetGameVersion(), newSession.GetContentFolder());
        }
    } else {
        //NOTE_SK: can't proceed w/o session
        this->close();
    }
}

void MainWindow::OnRibbonTabChanged(const MainRibbon::TabType tab) {
    if (MainRibbon::TabType::Model == tab) {
        ui->toolbox->hide();
        ui->splitterSidePanel->show();
        mModelHierarchyTree->show();
        mModelPropertyBrowser->show();
    } else if (MainRibbon::TabType::Skeleton == tab) {
        ui->splitterSidePanel->hide();
        mModelHierarchyTree->hide();
        mModelPropertyBrowser->hide();
        ui->toolbox->show();

        this->OnSkeletonShowBones(true);
        this->OnSkeletonShowBonesLinks(true);
    }
}

void MainWindow::OnImportMetroModel() {
    const fs::path& gameFolder = MetroContext::Get().GetGameFolderPath();
    fs::path meshesFolder = gameFolder / MetroFileSystem::Paths::MeshesFolder;

    QString name = QFileDialog::getOpenFileName(this, tr("Choose Metro model/mesh file..."), QString::fromStdWString(meshesFolder.wstring()), tr("Metro model files (*.model);;Metro mesh files (*.mesh);;All files (*.*)"));
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

void MainWindow::OnImportFBXModel() {
    QString name = QFileDialog::getOpenFileName(this, tr("Choose FBX model file..."), QString(), tr("FBX model files (*.fbx);;All files (*.*)"));
    if (!name.isEmpty()) {
        fs::path fullPath = name.toStdWString();

        ImporterFBX importer;
        importer.SetGameVersion(MetroContext::Get().GetGameVersion());
        RefPtr<MetroModelBase> model = importer.ImportModel(fullPath);
        if (model && mRenderPanel) {
            mRenderPanel->SetModel(model);

            this->UpdateUIForTheModel(model.get());
        }
    }
}

void MainWindow::OnExportMetroModel() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        QString name = QFileDialog::getSaveFileName(this, tr("Where to export Metro model..."), QString(), tr("Metro Model file (*.model);;All files (*.*)"));
        if (!name.isEmpty()) {
            ExportModelDlg dlg(this);
            dlg.SetModel(model.get());
            if (QDialog::Accepted == dlg.exec()) {
                fs::path fullPath = name.toStdWString();

                MetroModelSaveParams params;
                params.dstFile = fullPath;
                if (dlg.IsOverrideModelVersion()) {
                    params.gameVersion = scast<MetroGameVersion>(dlg.GetOverrideModelVersion());
                    params.saveFlags |= MetroModelSaveParams::SaveFlags::SaveForGameVersion;
                }
                if (dlg.IsExportMeshesInlined()) {
                    params.saveFlags |= MetroModelSaveParams::SaveFlags::InlineMeshes;
                }
                if (dlg.IsExportSkeletonInlined()) {
                    params.saveFlags |= MetroModelSaveParams::SaveFlags::InlineSkeleton;
                }

                MemWriteStream stream;
                if (model->Save(stream, params)) {
                    OSWriteFile(fullPath, stream.Data(), stream.GetWrittenBytesCount());
                }
            }
        }
    }
}

void MainWindow::OnExportOBJModel() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        QString name = QFileDialog::getSaveFileName(this, tr("Where to export OBJ model..."), QString(), tr("OBJ model file (*.obj);;All files (*.*)"));
        if (!name.isEmpty()) {
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
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        QString name = QFileDialog::getSaveFileName(this, tr("Where to export FBX model..."), QString(), tr("FBX model file (*.fbx);;All files (*.*)"));
        if (!name.isEmpty()) {
            fs::path fullPath = name.toStdWString();

            ExporterFBX expFbx;
            expFbx.SetExportMesh(true);
            expFbx.SetExportSkeleton(true);
            expFbx.SetExcludeCollision(true);
            expFbx.SetExportAnimation(false);

            expFbx.SetExporterName("MetroME");
            expFbx.SetTexturesExtension(".tga");
            expFbx.ExportModel(*model, fullPath);
        }
    }
}

void MainWindow::OnExportGLTFModel() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        QString name = QFileDialog::getSaveFileName(this, tr("Where to export GLTF model..."), QString(), tr("FBX model file (*.gltf);;All files (*.*)"));
        if (!name.isEmpty()) {
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
void MainWindow::OnImportMetroSkeleton() {
    const CharString& skelExt = MetroContext::Get().GetSkeletonExtension();
    QString filter = QString("Metro skeleton file (*%1);;All files (*.*)").arg(QString::fromStdString(skelExt));
    QString name = QFileDialog::getOpenFileName(this, tr("Select Metro skeleton to import..."), QString(), filter);
    if (!name.isEmpty()) {
        MemStream stream = OSReadFile(name.toStdWString());

        const bool is2033 = MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033;
        bool loaded = false;
        RefPtr<MetroSkeleton> skeleton = MakeRefPtr<MetroSkeleton>();
        if (is2033) {
            loaded = skeleton->LoadFromData_2033(stream);
        } else {
            loaded = skeleton->LoadFromData(stream);
        }

        if (loaded) {
            RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
            const bool newModel = (model == nullptr);
            if (newModel) {
                model = MakeRefPtr<MetroModelSkeleton>();
            }

            SCastRefPtr<MetroModelSkeleton>(model)->SetSkeleton(skeleton);

            if (newModel) {
                AABBox bbox = skeleton->CalcBBox();
                BSphere bsphere = { bbox.Center(), Max3(bbox.Extent()) };
                model->SetBBox(bbox);
                model->SetBSphere(bsphere);
                mRenderPanel->SetModel(model);
            }

            this->UpdateUIForTheModel(model.get());
        } else {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to load skeleton file!"));
        }
    }
}

void MainWindow::OnImportFBXSkeleton() {

}

void MainWindow::OnExportMetroSkeleton() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model && model->IsSkeleton()) {
        RefPtr<MetroModelSkeleton> skelModel = SCastRefPtr<MetroModelSkeleton>(model);
        RefPtr<MetroSkeleton> skeleton = skelModel->GetSkeleton();
        if (skeleton) {
            const bool is2033 = MetroContext::Get().GetGameVersion() == MetroGameVersion::OG2033;
            const CharString& skelExt = MetroContext::Get().GetSkeletonExtension();
            QString filter = QString("Metro skeleton file (*%1);;All files (*.*)").arg(QString::fromStdString(skelExt));

            QString name = QFileDialog::getSaveFileName(this, tr("Where to export Metro skeleton..."), QString(), filter);
            if (!name.isEmpty()) {
                fs::path fullPath = name.toStdWString();

                MemWriteStream stream;
                if (is2033) {
                    skeleton->Save_2033(stream);
                } else {
                    skeleton->Save(stream);
                }
                OSWriteFile(fullPath, stream.Data(), stream.GetWrittenBytesCount());
            }
        }
    }
}

void MainWindow::OnExportFBXSkeleton() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model && model->IsSkeleton()) {
        RefPtr<MetroModelSkeleton> skelModel = SCastRefPtr<MetroModelSkeleton>(model);
        RefPtr<MetroSkeleton> skeleton = skelModel->GetSkeleton();
        if (skeleton) {
            QString name = QFileDialog::getSaveFileName(this, tr("Where to export FBX file..."), QString(), tr("FBX model file (*.fbx);;All files (*.*)"));
            if (!name.isEmpty()) {
                fs::path fullPath = name.toStdWString();

                ExporterFBX expFbx;
                expFbx.SetExportMesh(false);
                expFbx.SetExportSkeleton(true);
                expFbx.SetExcludeCollision(true);
                expFbx.SetExportAnimation(false);

                expFbx.SetExporterName("MetroME");
                expFbx.SetTexturesExtension(".tga");
                expFbx.ExportModel(*model, fullPath);
            }
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
