#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderpanel.h"
#include "sessionsdlg.h"
#include "exportmodeldlg.h"
#include "exportfbxdlg.h"
#include "edittpresetsdlg.h"

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

#if 1
#include "metro/physics/MetroPhysics.h"
#define PHYSX2UTILS_IMPORT 1
#include "physx/physx2utils/physx2utils.h"
#define PHYSX3UTILS_IMPORT 1
#include "physx/physx3utils/physx3utils.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include "Mathematics/ContOrientedBox3.h"

#include "engine/DebugGeo.h"

static RefPtr<u4a::DebugGeo> DebugGeoFromCForm(MetroPhysicsCForm* phys, const MetroGameVersion gameVersion) {
    RefPtr<u4a::DebugGeo> debugGeo;

    std::function<IPhysXUtils*()> createPhysXUtils;
    std::function<void(IPhysXUtils*)> destroyPhysXUtils;

    if (gameVersion <= MetroGameVersion::OGLastLight) {
        createPhysXUtils = []()->IPhysXUtils* {
            IPhysX2Utils* utils2 = nullptr;
            if (nux2::CreatePhysX2Utils(0, &utils2)) {
                return utils2;
            } else {
                return nullptr;
            }
        };
        destroyPhysXUtils = [](IPhysXUtils* utils) {
            nux2::DestroyPhysX2Utils(scast<IPhysX2Utils*>(utils));
        };
    } else {
        createPhysXUtils = []()->IPhysXUtils* {
            IPhysX3Utils* utils3 = nullptr;
            if (nux3::CreatePhysX3Utils(0, &utils3)) {
                return utils3;
            } else {
                return nullptr;
            }
        };
        destroyPhysXUtils = [](IPhysXUtils* utils) {
            nux3::DestroyPhysX3Utils(scast<IPhysX3Utils*>(utils));
        };
    }

    IPhysXUtils* physxUtils = createPhysXUtils();
    if (physxUtils) {
        debugGeo = MakeRefPtr<u4a::DebugGeo>();
        bool hasSections = false;

        for (size_t i = 0, numMeshes = phys->GetNumCMeshes(); i < numMeshes; ++i) {
            RefPtr<MetroPhysicsCMesh> cmesh = phys->GetCMesh(i);

            BytesArray& cookedData = cmesh->GetCookedData();
            MyArray<vec3> vertices;
            MyArray<uint16_t> indices;
            if (!physxUtils->CookedTriMeshToRawVertices(cookedData.data(), cookedData.size(), vertices, indices)) {
                assert(false);
            } else {
                debugGeo->AddSection(vertices.data(), vertices.size(), indices.data(), indices.size());
                hasSections = true;
            }
        }

        if (!hasSections || !debugGeo->Create()) {
            debugGeo = nullptr;
        }

        destroyPhysXUtils(physxUtils);
    }

    return debugGeo;
}

RefPtr<MetroPhysicsCForm> MakeCFormFromModel(MetroModelBase* model, const MetroGameVersion gameVersion) {
    RefPtr<MetroPhysicsCForm> result;

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);

    std::function<IPhysXUtils* ()> createPhysXUtils;
    std::function<void(IPhysXUtils*)> destroyPhysXUtils;

    if (gameVersion <= MetroGameVersion::OGLastLight) {
        createPhysXUtils = []()->IPhysXUtils* {
            IPhysX2Utils* utils2 = nullptr;
            if (nux2::CreatePhysX2Utils(nux2::kInitCooker, &utils2)) {
                return utils2;
            } else {
                return nullptr;
            }
        };
        destroyPhysXUtils = [](IPhysXUtils* utils) {
            nux2::DestroyPhysX2Utils(scast<IPhysX2Utils*>(utils));
        };
    } else {
        createPhysXUtils = []()->IPhysXUtils* {
            IPhysX3Utils* utils3 = nullptr;
            if (nux3::CreatePhysX3Utils(nux3::kInitCooker, &utils3)) {
                return utils3;
            } else {
                return nullptr;
            }
        };
        destroyPhysXUtils = [](IPhysXUtils* utils) {
            nux3::DestroyPhysX3Utils(scast<IPhysX3Utils*>(utils));
        };
    }

    IPhysXUtils* physxUtils = createPhysXUtils();
    if (physxUtils) {
        bool failed = false;
        result = MakeRefPtr<MetroPhysicsCForm>();
        for (const MetroModelGeomData& gd : gds) {
            MyArray<vec3> points(gd.mesh->verticesCount);
            const VertexStatic* srcVertices = rcast<const VertexStatic*>(gd.vertices);
            for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
                points[i] = srcVertices[i].pos;
            }

            BytesArray cookedMesh;
            if (!physxUtils->RawVerticesToCookedTriMesh(points.data(), points.size(), rcast<const uint16_t*>(gd.faces), scast<size_t>(gd.mesh->facesCount) * 3, cookedMesh)) {
                failed = true;
                break;
            }

            RefPtr<MetroPhysicsCMesh> cmesh = MakeRefPtr<MetroPhysicsCMesh>();
            cmesh->SetShader(gd.model->GetMaterialString(0));
            cmesh->SetTexture(gd.model->GetMaterialString(1));
            cmesh->SetMaterialName1(gd.model->GetMaterialString(2));
            cmesh->SetMaterialName2(gd.model->GetMaterialString(3));
            cmesh->SetCookedData(cookedMesh.data(), cookedMesh.size());

            result->AddCMesh(cmesh);
        }

        if (failed) {
            result = nullptr;
        }

        destroyPhysXUtils(physxUtils);
    }

    return result;
}
#endif

#include "../MetroSessions.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mRenderPanel(nullptr)
    // Model rollouts
    , mModelMeshesRollout(new ModelMeshesRollout)
    , mModelPhysXRollout(new ModelPhysXRollout)
    // Skeleton rollouts
    , mBonesListRollout(new BonesListRollout)
    , mMotionsRollout(new MotionsRollout)
    , mFaceFXRollout(new FaceFXRollout)
    , mParamsRollout(new ParamsRollout)
    //
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
    connect(ui->ribbon, &MainRibbon::SignalModelTPresetChanged, this, &MainWindow::OnTPresetChanged);
    connect(ui->ribbon, &MainRibbon::SignalModelTPresetEditClicked, this, &MainWindow::OnTPresetsEdit);
    connect(ui->ribbon, &MainRibbon::SignalModelCalculateAOClicked, this, &MainWindow::OnCalculateAO);
    connect(ui->ribbon, &MainRibbon::SignalModelBuildLODsClicked, this, &MainWindow::OnBuildLODs);
    //
    connect(ui->ribbon, &MainRibbon::SignalSkeletonBuildBonesOBBsClicked, this, &MainWindow::OnSkeletonBuildOBBs);
    //
    connect(ui->ribbon, &MainRibbon::SignalPhysicsBuildClicked, this, &MainWindow::OnPhysicsBuild);
    //
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBoundsChecked, this, &MainWindow::OnShowBounds);
    connect(ui->ribbon, &MainRibbon::Signal3DViewBoundsTypeChanged, this, &MainWindow::OnBoundsTypeChanged);
    connect(ui->ribbon, &MainRibbon::Signal3DViewSubmodelsBoundsChecked, this, &MainWindow::OnSubmodelBounds);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesChecked, this, &MainWindow::OnSkeletonShowBones);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesLinksChecked, this, &MainWindow::OnSkeletonShowBonesLinks);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowBonesNamesChecked, this, &MainWindow::OnSkeletonShowBonesNames);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowModelChecked, this, &MainWindow::OnShowModel);
    connect(ui->ribbon, &MainRibbon::Signal3DViewModelLODValueChanged, this, &MainWindow::OnModelLODValueChanged);
    connect(ui->ribbon, &MainRibbon::Signal3DViewShowPhysicsChecked, this, &MainWindow::OnShowPhysics);
    connect(ui->ribbon, &MainRibbon::Signal3DViewRendererTypeChanged, this, &MainWindow::OnRendererTypeChanged);

    mRenderPanel = new RenderPanel(ui->renderContainer);
    mRenderPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRenderPanel->setGeometry(QRect(0, 0, ui->renderContainer->width(), ui->renderContainer->height()));
    ui->renderContainer->layout()->addWidget(mRenderPanel);

    // add model rollouts
    connect(mModelMeshesRollout, &ModelMeshesRollout::SignalMeshSelectionChanged, this, &MainWindow::OnModelMeshSelectionChanged);
    connect(mModelMeshesRollout, &ModelMeshesRollout::SignalMeshPropertiesChanged, this, &MainWindow::OnModelMeshPropertiesChanged);
    ui->toolboxModel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolboxModel->addWidget(mModelMeshesRollout);
    ui->toolboxModel->addWidget(mModelPhysXRollout);

    // add skeleton rollouts
    ui->toolboxSkeleton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolboxSkeleton->addWidget(mBonesListRollout);
    ui->toolboxSkeleton->addWidget(mMotionsRollout);
    ui->toolboxSkeleton->addWidget(mFaceFXRollout);
    ui->toolboxSkeleton->addWidget(mParamsRollout);
    ui->toolboxSkeleton->hide();

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
    // Setup model rollouts
    mModelMeshesRollout->FillForTheModel(model);
    mModelPhysXRollout->FillForTheModel(model);

    // Setup skeleton rollouts
    MetroSkeleton* skeleton = model->IsSkeleton() ? scast<MetroModelSkeleton*>(model)->GetSkeleton().get() : nullptr;
    mBonesListRollout->FillForTheSkeleton(skeleton);
    mMotionsRollout->FillForTheSkeleton(skeleton);
    mFaceFXRollout->FillForTheSkeleton(skeleton);
    mParamsRollout->FillForTheSkeleton(skeleton);

    ui->ribbon->SetLODLimit(scast<int>(model->GetLodCount()));

    MetroModelHierarchy* hierarchy = model->IsHierarchy() ? scast<MetroModelHierarchy*>(model) : nullptr;
    if (hierarchy && hierarchy->GetNumTPresets() > 0) {
        StringArray presets; presets.reserve(hierarchy->GetNumTPresets());
        for (size_t i = 0; i < hierarchy->GetNumTPresets(); ++i) {
            const MetroModelTPreset& tpreset = hierarchy->GetTPreset(i);
            presets.push_back(tpreset.name);
        }
        ui->ribbon->SetPresets(presets);
    } else {
        ui->ribbon->SetPresets({});
    }
}

void MainWindow::UpdatePhysicsFromTheModel(MetroModelBase* model, const fs::path& modelPath) {
    mModelPhysx = nullptr;

    if (model->IsHierarchy() && !model->IsSkinnedHierarchy()) {
        fs::path physPath = modelPath;
        const CharString& cformExt = MetroContext::Get().GetCFormExtension();
        physPath.replace_extension(cformExt);
        MemStream physStream = OSReadFile(physPath);
        if (physStream) {
            RefPtr<MetroPhysicsCForm> cform = MetroPhysicsLoadCFormFromStream(physStream, false);
            this->UpdatePhysicsFromCForm(cform);
        }
    }
}

void MainWindow::UpdatePhysicsFromCForm(RefPtr<MetroPhysicsCForm>& cform) {
    mModelPhysx = cform;
    if (mModelPhysx) {
        RefPtr<u4a::DebugGeo> debugGeo = DebugGeoFromCForm(mModelPhysx.get(), MetroContext::Get().GetGameVersion());
        mRenderPanel->SetDebugGeo(debugGeo);
    } else {
        mRenderPanel->SetDebugGeo(nullptr);
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
        ui->toolboxSkeleton->hide();
        ui->toolboxModel->show();
    } else if (MainRibbon::TabType::Skeleton == tab) {
        ui->toolboxModel->hide();
        ui->toolboxSkeleton->show();
    } else if (MainRibbon::TabType::Physics == tab) {
        ui->toolboxModel->hide();
        ui->toolboxSkeleton->hide();
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
                this->UpdatePhysicsFromTheModel(model.get(), fullPath);
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
            dlg.SetModel(model.get(), mModelPhysx != nullptr);
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
            ExportFBXDlg dlg(this);
            dlg.SetModel(model.get());
            if (QDialog::Accepted == dlg.exec()) {
                fs::path fullPath = name.toStdWString();

                ExporterFBX expFbx;
                expFbx.SetExportMesh(true);
                expFbx.SetExportSkeleton(dlg.GetExportSkeleton());
                expFbx.SetExportShadowGeometry(dlg.GetExportShadowGeometry());
                expFbx.SetExportLODs(dlg.GetExportLODs());
                expFbx.SetExcludeCollision(true);
                expFbx.SetExportAnimation(false);

                expFbx.SetExporterName("MetroME");
                expFbx.SetTexturesExtension(".tga");
                expFbx.ExportModel(*model, fullPath);
            }
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
void MainWindow::OnTPresetChanged(int index) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;

    if (model && model->IsHierarchy()) {
        RefPtr<MetroModelHierarchy> hierarchy = SCastRefPtr<MetroModelHierarchy>(model);

        if (index <= 0) {
            hierarchy->ResetTPreset();
        } else {
            const MetroModelTPreset& tpreset = hierarchy->GetTPreset(index - 1);
            hierarchy->ApplyTPreset(tpreset.name);
        }

        if (mRenderPanel) {
            mRenderPanel->UpdateModelProps();
        }
    }
}

void MainWindow::OnTPresetsEdit() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;

    if (model && model->IsHierarchy()) {
        RefPtr<MetroModelHierarchy> hierarchy = SCastRefPtr<MetroModelHierarchy>(model);

        EditTPresetsDlg dlg(this);
        dlg.SetModel(hierarchy);
        dlg.exec();
    }
}

extern "C++" bool CalculateModelAO(RefPtr<MetroModelBase>& model, const size_t quality);
void MainWindow::OnCalculateAO(int quality) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        if (CalculateModelAO(model, scast<size_t>(quality))) {
            this->OnModelMeshPropertiesChanged();
        }
    }
}

extern "C++" RefPtr<MetroModelBase> BuildModelLOD(RefPtr<MetroModelBase>&model, const float ratio);
void MainWindow::OnBuildLODs() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        if (model->IsSkeleton() || model->IsSkinnedHierarchy() || model->IsSoft()) {
            QMessageBox::critical(this, this->windowTitle(), tr("Only static models supported at the moment!"));
        } else {
            RefPtr<MetroModelHierarchy> hierarchy = SCastRefPtr<MetroModelHierarchy>(model);
            hierarchy->RemoveLODs();

            const size_t numChildren = hierarchy->GetChildrenCount();

            for (size_t i = 0; i < 2; ++i) {
                const float ratio = (i == 0) ? 0.6f : 0.3f;

                MyArray<RefPtr<MetroModelBase>> lodMeshes; lodMeshes.reserve(numChildren);
                for (size_t i = 0; i < numChildren; ++i) {
                    RefPtr<MetroModelBase> child = hierarchy->GetChild(i);
                    if (!child->IsCollisionModel()) {
                        RefPtr<MetroModelBase> lodMesh = BuildModelLOD(child, ratio);
                        if (lodMesh) {
                            lodMeshes.push_back(lodMesh);
                        }
                    }
                }

                if (!lodMeshes.empty()) {
                    RefPtr<MetroModelHierarchy> lod = MakeRefPtr<MetroModelHierarchy>();
                    for (auto& lm : lodMeshes) {
                        lod->AddChild(lm);
                    }

                    hierarchy->AddLOD(lod);
                }
            }

            this->OnModelMeshPropertiesChanged();
        }
    }
}

//
void MainWindow::OnPhysicsBuild(int physicsSource) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        if (physicsSource == 1) {
            if (model->GetLodCount() == 0) {
                const int answer = QMessageBox::question(this, this->windowTitle(), tr("The model doesn't have LODs, use main geometry instead?"), QMessageBox::Yes, QMessageBox::No);
                if (QMessageBox::No == answer) {
                    return;
                }
            } else {
                model = model->GetLod(model->GetLodCount() - 1);
            }
        }

        auto cform = MakeCFormFromModel(model.get(), MetroContext::Get().GetGameVersion());
        this->UpdatePhysicsFromCForm(cform);
    }
}

//
void MainWindow::OnSkeletonBuildOBBs() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model && model->IsSkeleton()) {
        RefPtr<MetroModelSkeleton> skelModel = SCastRefPtr<MetroModelSkeleton>(model);
        const RefPtr<MetroSkeleton>& skeleton = skelModel->GetSkeleton();

        const size_t numChildren = skelModel->GetChildrenCount();
        for (size_t i = 0; i < numChildren; ++i) {
            RefPtr<MetroModelBase> child = skelModel->GetChild(i);
            RefPtr<MetroModelMesh> mesh = child->GetMesh();
            const size_t numVertices = mesh->verticesCount;
            const VertexSkinned* vertices = rcast<const VertexSkinned*>(child->GetVerticesMemData());
            MyArray<MyArray<vec3>> perBoneVertices(mesh->bonesRemap.size());
            for (size_t j = 0; j < numVertices; ++j) {
                const size_t boneIdx = vertices[j].bones[2] / 3;    // ZYXW swizzle
                mat4 boneInvTransform = skeleton->GetBoneFullTransformInv(mesh->bonesRemap[boneIdx]);
                vec3 vertexPos = boneInvTransform * vec4(DecodeSkinnedPosition(vertices[j].pos) * mesh->verticesScale, 1.0f);
                perBoneVertices[boneIdx].push_back(vertexPos);
            }

            MyArray<OBBox> obbs;
            for (const MyArray<vec3>& boneVerts : perBoneVertices) {
                gte::OrientedBox3<float> box3;
                OBBox bbox; bbox.Reset();
                if (!boneVerts.empty() && gte::GetContainer<float>(scast<int>(boneVerts.size()), rcast<const gte::Vector3<float>*>(boneVerts.data()), box3)) {
                    bbox.matrix[0] = *rcast<vec3*>(&box3.axis[0]);
                    bbox.matrix[1] = *rcast<vec3*>(&box3.axis[1]);
                    bbox.matrix[2] = *rcast<vec3*>(&box3.axis[2]);
                    bbox.offset = *rcast<vec3*>(&box3.center);
                    bbox.hsize = *rcast<vec3*>(&box3.extent);
                }
                obbs.push_back(bbox);
            }

            SCastRefPtr<MetroModelSkin>(child)->SetBonesOBB(obbs);
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

void MainWindow::OnShowModel(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetShowModel(checked);
    }
}

void MainWindow::OnModelLODValueChanged(int value) {
    if (mRenderPanel) {
        mRenderPanel->SetLod(scast<size_t>(value));
    }
}

void MainWindow::OnShowPhysics(bool checked) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugShowPhysics(checked);
    }
}

void MainWindow::OnRendererTypeChanged(MainRibbon::RendererType type) {
    if (mRenderPanel) {
        mRenderPanel->SetRendererType(scast<size_t>(type));
    }
}

void MainWindow::OnModelMeshSelectionChanged(int) {
}

void MainWindow::OnModelMeshPropertiesChanged() {
    if (mRenderPanel) {
        mRenderPanel->UpdateModelProps();
    }
}
