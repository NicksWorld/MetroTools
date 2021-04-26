#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderpanel.h"

#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>

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

    bool deviceIsOk = u4a::Renderer::Get().CreateDevice();
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
}

MainWindow::~MainWindow() {
    delete ui;
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

                //this->UpdateUIForTheModel(model.get());
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

            //this->UpdateUIForTheModel(model.get());
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
