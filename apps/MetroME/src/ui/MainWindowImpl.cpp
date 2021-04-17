#include "MainWindowImpl.h"

#include "RenderPanel.h"

#include <msclr/marshal_cppstd.h>

namespace MetroEX {
    inline fs::path StringToPath(System::String^ s) {
        return msclr::interop::marshal_as<std::wstring>(s);
    }
    inline System::String^ PathToString(const fs::path& p) {
        return msclr::interop::marshal_as<System::String^>(p.native());
    }

    inline System::String^ ToNetString(const CharString& s) {
        return msclr::interop::marshal_as<System::String^>(s);
    }
    inline System::String^ ToNetString(const WideString& s) {
        return msclr::interop::marshal_as<System::String^>(s);
    }

    inline CharString NetToCharStr(System::String^ s) {
        return msclr::interop::marshal_as<CharString>(s);
    }
    inline WideString NetToWideStr(System::String^ s) {
        return msclr::interop::marshal_as<WideString>(s);
    }
}

#include "metro/MetroContext.h"
#include "metro/MetroModel.h"

#include "exporters/ExporterOBJ.h"
#include "exporters/ExporterFBX.h"

#include "importers/ImporterOBJ.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

MainWindowImpl::MainWindowImpl()
    : mWindow(nullptr)
    , mRenderPanel(nullptr)
    , mSelectedGD(-1)
{
}
MainWindowImpl::~MainWindowImpl() {
    MySafeDelete(mRenderPanel);
}


void MainWindowImpl::OnWindowLoaded(MetroMEControls::MainWindow^ wnd) {
    mWindow = wnd;

    if (u4a::Renderer::Get().CreateDevice()) {
        mRenderPanel = new RenderPanel();
        mRenderPanel->Initialize(mWindow->GetRenderPanelHwnd().ToPointer(),
                                 scast<size_t>(mWindow->GetRenderPanelWidth()),
                                 scast<size_t>(mWindow->GetRenderPanelHeight()));

        u4a::Renderer::Get().Initialize();
        u4a::ResourcesManager::Get().Initialize();
    }

    MetroContext::Get().InitFromContentFolder(R"(e:\Games\SteamLibrary\steamapps\common\Metro Last Light Redux\!!\extracted\content)");
}

void MainWindowImpl::OnUpdate() {
    if (mRenderPanel) {
        mRenderPanel->Draw();
    }
}

void MainWindowImpl::OnRenderPanelResized() {
    if (mRenderPanel) {
        mRenderPanel->OnResize(scast<size_t>(mWindow->GetRenderPanelWidth()),
                               scast<size_t>(mWindow->GetRenderPanelHeight()));
    }
}

void MainWindowImpl::OnRenderPanelMouseButton(bool left, bool right, float x, float y) {
    if (mRenderPanel) {
        mRenderPanel->OnMouseButton(left, right, x, y);
    }
}

void MainWindowImpl::OnRenderPanelMouseMove(float x, float y) {
    if (mRenderPanel) {
        mRenderPanel->OnMouseMove(x, y);
    }
}

void MainWindowImpl::OnRenderPanelMouseWheel(float delta) {
    if (mRenderPanel) {
        mRenderPanel->OnMouseWheel(delta);
    }
}

void MainWindowImpl::OnTreeViewSelectionChanged(int selectedTag) {
    if (selectedTag >= 0 && mRenderPanel) {
        RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
        if (model) {
            MyArray<MetroModelGeomData> gds;
            model->CollectGeomData(gds);

            const MetroModelBase* gdModel = gds[selectedTag].model;
            System::Collections::Generic::List<System::Tuple<System::String^, System::String^>^>^ lst = gcnew System::Collections::Generic::List<System::Tuple<System::String^, System::String^>^>();
            lst->Add(gcnew System::Tuple<System::String^, System::String^>("texture", MetroEX::ToNetString(gdModel->GetMaterialString(0))));
            lst->Add(gcnew System::Tuple<System::String^, System::String^>("shader", MetroEX::ToNetString(gdModel->GetMaterialString(1))));
            lst->Add(gcnew System::Tuple<System::String^, System::String^>("material", MetroEX::ToNetString(gdModel->GetMaterialString(2))));
            lst->Add(gcnew System::Tuple<System::String^, System::String^>("src_mat", MetroEX::ToNetString(gdModel->GetMaterialString(3))));

            mWindow->ModelPropsSetArray(lst);

            mSelectedGD = selectedTag;
        }
    }
}

void MainWindowImpl::OnModelPropChanged(System::String^ propName, System::String^ propValue) {
    if (mSelectedGD >= 0 && mRenderPanel) {
        RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
        if (model) {
            MyArray<MetroModelGeomData> gds;
            model->CollectGeomData(gds);

            MetroModelBase* gdModel = const_cast<MetroModelBase*>(gds[mSelectedGD].model);

            size_t idx = 0;
            if (propName->Equals("texture")) {
                idx = 0;
            } else if (propName->Equals("shader")) {
                idx = 1;
            } else if (propName->Equals("material")) {
                idx = 2;
            } else if (propName->Equals("src_mat")) {
                idx = 3;
            }

            CharString newStr = MetroEX::NetToCharStr(propValue);
            const CharString& srcStr = gdModel->GetMaterialString(idx);

            if (newStr != srcStr) {
                gdModel->SetMaterialString(newStr, idx);

                mRenderPanel->UpdateModelProps();
            }
        }
    }
}

void MainWindowImpl::OnFileImportMetroModelCommand(System::String^ filePath) {
    fs::path fullPath = MetroEX::StringToPath(filePath);

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

void MainWindowImpl::OnFileImportOBJModelCommand(System::String^ filePath) {
    fs::path fullPath = MetroEX::StringToPath(filePath);

    ImporterOBJ importer;
    RefPtr<MetroModelBase> model = importer.ImportModel(fullPath);
    if (model && mRenderPanel) {
        mRenderPanel->SetModel(model);

        this->UpdateUIForTheModel(model.get());
    }
}

void MainWindowImpl::OnFileExportMetroModelCommand(System::String^ filePath) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        MemWriteStream stream;
        if (model->Save(stream)) {
            fs::path fullPath = MetroEX::StringToPath(filePath);
            OSWriteFile(fullPath, stream.Data(), stream.GetWrittenBytesCount());
        }
    }
}

void MainWindowImpl::OnFileExportOBJModelCommand(System::String^ filePath) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        fs::path fullPath = MetroEX::StringToPath(filePath);

        ExporterOBJ expObj;
        expObj.SetExcludeCollision(true);
        expObj.SetExporterName("MetroME");
        expObj.SetTexturesExtension(".tga");
        expObj.ExportModel(*model, fullPath);
    }
}

void MainWindowImpl::OnFileExportFBXModelCommand(System::String^ filePath) {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    if (model) {
        fs::path fullPath = MetroEX::StringToPath(filePath);

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


// debug
void MainWindowImpl::OnDebugShowBounds(bool show) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugShowBounds(show);
    }
}

void MainWindowImpl::OnDebugShowSubmodelsBounds(bool show) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugShowSubmodelsBounds(show);
    }
}

void MainWindowImpl::OnDebugBoundsTypeChanged(System::String^ newType) {
    if (mRenderPanel) {
        const CharString typeName = MetroEX::NetToCharStr(newType);
        if (typeName == "Box") {
            mRenderPanel->SetDebugBoundsType(RenderPanel::DebugBoundsType::Box);
        } else {
            mRenderPanel->SetDebugBoundsType(RenderPanel::DebugBoundsType::Sphere);
        }
    }
}

void MainWindowImpl::OnDebugSkeletonShowBones(bool show) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBones(show);
    }
}

void MainWindowImpl::OnDebugSkeletonShowBonesLinks(bool show) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBonesLinks(show);
    }
}

void MainWindowImpl::OnDebugSkeletonShowBonesNames(bool show) {
    if (mRenderPanel) {
        mRenderPanel->SetDebugSkeletonShowBonesNames(show);
    }
}


bool MainWindowImpl::CanExportModel() {
    RefPtr<MetroModelBase> model = mRenderPanel ? mRenderPanel->GetModel() : nullptr;
    return model != nullptr;
}


void MainWindowImpl::UpdateUIForTheModel(MetroModelBase* model) {
    mSelectedGD = -1;

    const MetroModelType mtype = model->GetModelType();
    RefPtr<MetroSkeleton> skeleton;

    if (mtype == MetroModelType::Skeleton ||
        mtype == MetroModelType::Skeleton2 ||
        mtype == MetroModelType::Skeleton3) {
        skeleton = scast<MetroModelSkeleton*>(model)->GetSkeleton();
    }

    mWindow->EnableSkeletonTab(skeleton != nullptr);

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);

    if (!gds.empty()) {
        mWindow->TreeViewReset("Model", -1);

        int idx = 0;
        for (const auto& gd : gds) {
            CharString gdName = CharString("Mesh_") + std::to_string(idx);
            mWindow->TreeViewAddSub(MetroEX::ToNetString(gdName), idx);

            ++idx;
        }

        mWindow->ModelPropsSetArray(nullptr);
    }
}
