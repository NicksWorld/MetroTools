#include "mainribbon.h"

#include <QHBoxLayout>
#include <QMenu>


#include "simpleribbon/SimpleRibbon.h"
#include "simpleribbon/SimpleRibbonTab.h"
#include "simpleribbon/SimpleRibbonGroup.h"
#include "simpleribbon/SimpleRibbonVBar.h"
#include "simpleribbon/SimpleRibbonButton.h"

constexpr const char* sRendererTypesNames[scast<size_t>(MainRibbon::RendererType::NumRendererTypes)] = {
    "Regular",
    "Wireframe",
    "Albedo",
    "Normal",
    "Gloss",
    "Roughness",
    "AO"
};

MainRibbon::MainRibbon(QWidget* parent)
    : QWidget(parent)
    , mRibbon(nullptr)
    // tabs
    , mTabModel(nullptr)
    , mTabSkeleton(nullptr)
    , mTabAnimation(nullptr)
    , mTab3DView(nullptr)
    // model groups
    , mGroupModelFile(nullptr)
    , mGroupModelPreset(nullptr)
    , mGroupModelAO(nullptr)
    // skeleton groups
    , mGroupSkeletonFile(nullptr)
    // physics groups
    , mGroupPhysicsTools(nullptr)
    // 3d view groups
    , mGroup3DViewBounds(nullptr)
    , mGroup3DViewSkeleton(nullptr)
    , mGroup3DViewModel(nullptr)
    , mGroup3DViewPhysics(nullptr)
    , mGroup3DViewRenderer(nullptr)
    // model controls
    , mComboTPreset(nullptr)
    , mCalculateAOButton(nullptr)
    // physics controls
    , mComboPhysicsSource(nullptr)
    , mBuildPhysicsButton(nullptr)
    // 3d view controls
    , mComboBoundsType(nullptr)
    , mCheckSubmodelBounds(nullptr)
    , mCheckShowBonesLinks(nullptr)
    , mCheckShowBonesNames(nullptr)
    , mCheckShowPhysics(nullptr)
    , mCheckShowModel(nullptr)
    , mModelLod(nullptr)
    , mRendererType(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->BuildRibbon();
}
MainRibbon::~MainRibbon() {
}

void MainRibbon::EnableTab(const TabType tab, const bool enable) {
    switch (tab) {
        case TabType::Model:
            mRibbon->EnableRibbonTab(mTabModel->GetTabName(), enable);
        break;
        case TabType::Skeleton:
            mRibbon->EnableRibbonTab(mTabSkeleton->GetTabName(), enable);
        break;
        case TabType::Animation:
            mRibbon->EnableRibbonTab(mTabAnimation->GetTabName(), enable);
        break;
        case TabType::View:
            mRibbon->EnableRibbonTab(mTab3DView->GetTabName(), enable);
        break;
    }
}

void MainRibbon::SetLODLimit(const int limit) {
    mModelLod->setMinimum(0);
    mModelLod->setMaximum(limit);
}

void MainRibbon::SetPresets(const StringArray& presets) {
    mComboTPreset->clear();

    if (presets.empty()) {
        mComboTPreset->setEnabled(false);
    } else {
        mComboTPreset->setEnabled(true);
        mComboTPreset->addItem("- None -"); // reset preset
        for (const CharString& p : presets) {
            mComboTPreset->addItem(QString::fromStdString(p));
        }
    }
}


void MainRibbon::OnCurrentTabChanged(int index) {
    emit SignalCurrentTabChanged(static_cast<TabType>(index));
}

void MainRibbon::OnFileImportMetroModelCommand(bool) {
    emit SignalFileImportMetroModel();
}

void MainRibbon::OnFileImportOBJModelCommand(bool) {
    emit SignalFileImportOBJModel();
}

void MainRibbon::OnFileImportFBXModelCommand(bool checked) {
    emit SignalFileImportFBXModel();
}

void MainRibbon::OnFileExportMetroModelCommand(bool) {
    emit SignalFileExportMetroModel();
}

void MainRibbon::OnFileExportOBJModelCommand(bool) {
    emit SignalFileExportOBJModel();
}

void MainRibbon::OnFileExportFBXModelCommand(bool) {
    emit SignalFileExportFBXModel();
}

void MainRibbon::OnFileExportGLTFModelCommand(bool) {
    emit SignalFileExportGLTFModel();
}

//
void MainRibbon::OnFileImportMetroSkeletonCommand(bool) {
    emit SignalFileImportMetroSkeleton();
}

void MainRibbon::OnFileImportFBXSkeletonCommand(bool) {
    emit SignalFileImportFBXSkeleton();
}

void MainRibbon::OnFileExportMetroSkeletonCommand(bool) {
    emit SignalFileExportMetroSkeleton();
}

void MainRibbon::OnFileExportFBXSkeletonCommand(bool) {
    emit SignalFileExportFBXSkeleton();
}

//
void MainRibbon::OnModelTPresetChanged(int index) {
    emit SignalModelTPresetChanged(index);
}

void MainRibbon::OnModelTPresetEditClicked() {
    emit SignalModelTPresetEditClicked();
}

void MainRibbon::OnModelCalculateAOClicked() {
    const int idx = mAOCalcQuality->currentIndex();
    emit SignalModelCalculateAOClicked(idx >= 0 ? idx : 0);
}


//
void MainRibbon::OnPhysicsBuildButtonClicked() {
    emit SignalPhysicsBuildClicked(mComboPhysicsSource->currentIndex());
}

//
void MainRibbon::On3DViewShowBoundsChecked(int state) {
    const bool checked = (Qt::Checked == state);

    mComboBoundsType->setEnabled(checked);
    mCheckSubmodelBounds->setEnabled(checked);

    emit Signal3DViewShowBoundsChecked(checked);
}

void MainRibbon::On3DViewBoundsTypeChanged(int index) {
    emit Signal3DViewBoundsTypeChanged(index);
}

void MainRibbon::On3DViewSubmodelsBoundsChecked(int state) {
    emit Signal3DViewSubmodelsBoundsChecked(Qt::Checked == state);
}

void MainRibbon::On3DViewShowBonesChecked(int state) {
    const bool checked = (Qt::Checked == state);

    mCheckShowBonesLinks->setEnabled(checked);
    mCheckShowBonesNames->setEnabled(checked);

    emit Signal3DViewShowBonesChecked(checked);
}

void MainRibbon::On3DViewShowBonesLinksChecked(int state) {
    emit Signal3DViewShowBonesLinksChecked(Qt::Checked == state);
}

void MainRibbon::On3DViewShowBonesNamesChecked(int state) {
    emit Signal3DViewShowBonesNamesChecked(Qt::Checked == state);
}

void MainRibbon::On3DViewShowModelChecked(int state) {
    emit Signal3DViewShowModelChecked(Qt::Checked == state);
}

void MainRibbon::On3DViewModelLODValueChanged(int value) {
    emit Signal3DViewModelLODValueChanged(value);
}

void MainRibbon::On3DViewShowPhysicsChecked(int state) {
    emit Signal3DViewShowPhysicsChecked(Qt::Checked == state);
}

void MainRibbon::On3DViewRendererTypeChanged(int index) {
    emit Signal3DViewRendererTypeChanged(index >= 0 ? scast<RendererType>(index) : RendererType::Regular);
}


void MainRibbon::BuildRibbon() {
    mRibbon = new SimpleRibbon(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(mRibbon->sizePolicy().hasHeightForWidth());
    mRibbon->setSizePolicy(sizePolicy);
    this->layout()->addWidget(mRibbon);

    mTabModel = mRibbon->AddRibbonTab(tr("Model"));
    mTabSkeleton = mRibbon->AddRibbonTab(tr("Skeleton"));
    mTabAnimation = mRibbon->AddRibbonTab(tr("Animation"));
    mTabPhysics = mRibbon->AddRibbonTab(tr("Physics"));
    mTab3DView = mRibbon->AddRibbonTab(tr("3D View"));

    this->BuildModelTab();
    this->BuildSkeletonTab();
    this->BuildAnimationTab();
    this->BuildPhysicsTab();
    this->Build3DViewTab();

    connect(mRibbon, &SimpleRibbon::currentChanged, this, &MainRibbon::OnCurrentTabChanged);
}

void MainRibbon::BuildModelTab() {
    mGroupModelFile = mTabModel->AddRibbonGroup(tr("File"));
    mGroupModelPreset = mTabModel->AddRibbonGroup(tr("Presets"));
    mGroupModelAO = mTabModel->AddRibbonGroup(tr("AO"));

    SimpleRibbonButton* importModelButton = new SimpleRibbonButton;
    importModelButton->SetText(tr("Import..."));
    importModelButton->SetTooltip(tr("Import model from file"));
    importModelButton->SetIcon(QPixmap(":/imgs/ImportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* importFromModelAction = menu->addAction(QPixmap(":/imgs/metro_icon.png"), tr("Import from Metro model..."));
        menu->addSeparator();
        QAction* importFromObjAction = menu->addAction(QPixmap(":/imgs/obj.svg"), tr("Import from OBJ model..."));
        menu->addSeparator();
        QAction* importFromFbxAction = menu->addAction(QPixmap(":/imgs/fbx.svg"), tr("Import from FBX model..."));
        importModelButton->SetMenu(menu);

        connect(importModelButton, &SimpleRibbonButton::clicked, this, [importModelButton](bool checked) {
            importModelButton->showMenu();
        });

        connect(importFromModelAction, &QAction::triggered, this, &MainRibbon::OnFileImportMetroModelCommand);
        connect(importFromObjAction, &QAction::triggered, this, &MainRibbon::OnFileImportOBJModelCommand);
        connect(importFromFbxAction, &QAction::triggered, this, &MainRibbon::OnFileImportFBXModelCommand);
    }
    mGroupModelFile->AddWidget(importModelButton);

    // Add 'Export' button
    SimpleRibbonButton* exportModelButton = new SimpleRibbonButton;
    exportModelButton->SetText(tr("Export..."));
    exportModelButton->SetTooltip(tr("Export model to file"));
    exportModelButton->SetIcon(QPixmap(":/imgs/ExportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* exportToModelAction = menu->addAction(QPixmap(":/imgs/metro_icon.png"), tr("Export to Metro model..."));
        menu->addSeparator();
        QAction* exportToOBJAction = menu->addAction(QPixmap(":/imgs/obj.svg"), tr("Export to OBJ model..."));
        menu->addSeparator();
        QAction* exportToFBXAction = menu->addAction(QPixmap(":/imgs/fbx.svg"), tr("Export to FBX model..."));
        menu->addSeparator();
        QAction* exportToGLTFAction = menu->addAction(QPixmap(":/imgs/gltf.svg"), tr("Export to GLTF model..."));
        exportModelButton->SetMenu(menu);

        connect(exportModelButton, &SimpleRibbonButton::clicked, this, [exportModelButton](bool checked) {
            exportModelButton->showMenu();
        });

        connect(exportToModelAction, &QAction::triggered, this, &MainRibbon::OnFileExportMetroModelCommand);
        connect(exportToOBJAction, &QAction::triggered, this, &MainRibbon::OnFileExportOBJModelCommand);
        connect(exportToFBXAction, &QAction::triggered, this, &MainRibbon::OnFileExportFBXModelCommand);
        connect(exportToGLTFAction, &QAction::triggered, this, &MainRibbon::OnFileExportGLTFModelCommand);
    }
    mGroupModelFile->AddWidget(exportModelButton);

    // tpresets
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QLabel* label = new QLabel();
        label->setText(tr("Apply preset:"));

        mComboTPreset = new QComboBox();
        mComboTPreset->setEditable(false);
        mComboTPreset->setEnabled(false);
        connect(mComboTPreset, &QComboBox::currentIndexChanged, this, &MainRibbon::OnModelTPresetChanged);

        QPushButton* editBtn = new QPushButton();
        editBtn->setText(tr("Edit..."));
        connect(editBtn, &QPushButton::clicked, this, &MainRibbon::OnModelTPresetEditClicked);

        vbar->AddWidget(label);
        vbar->AddWidget(mComboTPreset);
        vbar->AddWidget(editBtn);

        mGroupModelPreset->AddWidget(vbar);
    }

    // AO
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QLabel* label = new QLabel();
        label->setText(tr("AO calc quality:"));
        label->setAlignment(Qt::AlignHCenter);

        mAOCalcQuality = new QComboBox();
        mAOCalcQuality->addItem(tr("Low (fast)"));
        mAOCalcQuality->addItem(tr("Normal"));
        mAOCalcQuality->addItem(tr("High"));
        mAOCalcQuality->addItem(tr("Ultra"));
        mAOCalcQuality->setEditable(false);

        mCalculateAOButton = new QPushButton();
        mCalculateAOButton->setText(tr("Calculate"));
        connect(mCalculateAOButton, &QPushButton::clicked, this, &MainRibbon::OnModelCalculateAOClicked);

        vbar->AddWidget(label);
        vbar->AddWidget(mAOCalcQuality);
        vbar->AddWidget(mCalculateAOButton);
        mGroupModelAO->AddWidget(vbar);
    }
}

void MainRibbon::BuildSkeletonTab() {
#if 0
    mGroupSkeletonFile = mTabSkeleton->AddRibbonGroup(tr("File"));

    SimpleRibbonButton* importSkeletonButton = new SimpleRibbonButton;
    importSkeletonButton->SetText(tr("Import..."));
    importSkeletonButton->SetTooltip(tr("Import skeleton from file"));
    importSkeletonButton->SetIcon(QIcon(":/imgs/ImportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* importFromSkeletonAction = menu->addAction("Import from Metro skeleton...");
        menu->addSeparator();
        QAction* importFromFbxAction = menu->addAction("Import from FBX skeleton...");
        importSkeletonButton->SetMenu(menu);

        connect(importFromSkeletonAction, &QAction::triggered, this, &MainRibbon::OnFileImportMetroSkeletonCommand);
        connect(importFromFbxAction, &QAction::triggered, this, &MainRibbon::OnFileImportFBXSkeletonCommand);
    }
    mGroupSkeletonFile->AddWidget(importSkeletonButton);

    // Add 'Export' button
    SimpleRibbonButton* exportSkeletonButton = new SimpleRibbonButton;
    exportSkeletonButton->SetText(tr("Export..."));
    exportSkeletonButton->SetTooltip(tr("Export skeleton to file"));
    exportSkeletonButton->SetIcon(QIcon(":/imgs/ExportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* exportToSkeletonAction = menu->addAction("Export to Metro skeleton...");
        menu->addSeparator();
        QAction* exportToFBXAction = menu->addAction("Export to FBX skeleton...");
        exportSkeletonButton->SetMenu(menu);

        connect(exportToSkeletonAction, &QAction::triggered, this, &MainRibbon::OnFileExportMetroSkeletonCommand);
        connect(exportToFBXAction, &QAction::triggered, this, &MainRibbon::OnFileExportFBXSkeletonCommand);
    }
    mGroupSkeletonFile->AddWidget(exportSkeletonButton);
#endif
}

void MainRibbon::BuildAnimationTab() {
}

void MainRibbon::BuildPhysicsTab() {
    mGroupPhysicsTools = mTabPhysics->AddRibbonGroup(tr("Tools"));

    // tools
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QLabel* label = new QLabel();
        label->setText(tr("Build physics from:"));

        mComboPhysicsSource = new QComboBox();
        mComboPhysicsSource->addItem(tr("Model geometry"));
        mComboPhysicsSource->addItem(tr("Lowest LOD"));
        mComboPhysicsSource->setEditable(false);

        mBuildPhysicsButton = new QPushButton();
        mBuildPhysicsButton->setText(tr("Build physics"));
        mBuildPhysicsButton->setIcon(QPixmap(":/imgs/hammer.svg"));
        connect(mBuildPhysicsButton, &QPushButton::clicked, this, &MainRibbon::OnPhysicsBuildButtonClicked);

        vbar->AddWidget(label);
        vbar->AddWidget(mComboPhysicsSource);
        vbar->AddWidget(mBuildPhysicsButton);
        mGroupPhysicsTools->AddWidget(vbar);
    }

    // file
    {
    }
}

void MainRibbon::Build3DViewTab() {
    mGroup3DViewBounds = mTab3DView->AddRibbonGroup(tr("Bounds"));
    mGroup3DViewSkeleton = mTab3DView->AddRibbonGroup(tr("Skeleton"));
    mGroup3DViewModel = mTab3DView->AddRibbonGroup(tr("Model"));
    mGroup3DViewPhysics = mTab3DView->AddRibbonGroup(tr("Physics"));
    mGroup3DViewRenderer = mTab3DView->AddRibbonGroup(tr("Renderer"));

    // Bounds
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QCheckBox* chkShowBounds = new QCheckBox();
        chkShowBounds->setText(tr("Show bounds"));
        connect(chkShowBounds, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowBoundsChecked);

        mComboBoundsType = new QComboBox();
        mComboBoundsType->addItem(QIcon(":/imgs/BBox.png"), tr("Box"));
        mComboBoundsType->addItem(QIcon(":/imgs/BSphere.png"), tr("Sphere"));
        mComboBoundsType->setEditable(false);
        mComboBoundsType->setEnabled(false);
        connect(mComboBoundsType, &QComboBox::currentIndexChanged, this, &MainRibbon::On3DViewBoundsTypeChanged);

        mCheckSubmodelBounds = new QCheckBox();
        mCheckSubmodelBounds->setText(tr("Submodels bounds"));
        mCheckSubmodelBounds->setEnabled(false);
        connect(mCheckSubmodelBounds, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewSubmodelsBoundsChecked);

        vbar->AddWidget(chkShowBounds);
        vbar->AddWidget(mComboBoundsType);
        vbar->AddWidget(mCheckSubmodelBounds);
        mGroup3DViewBounds->AddWidget(vbar);
    }
    // Skeleton
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QCheckBox* chkShowBones = new QCheckBox();
        chkShowBones->setText(tr("Show bones"));
        connect(chkShowBones, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowBonesChecked);

        mCheckShowBonesLinks = new QCheckBox();
        mCheckShowBonesLinks->setText(tr("Show bones links"));
        connect(mCheckShowBonesLinks, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowBonesLinksChecked);

        mCheckShowBonesNames = new QCheckBox();
        mCheckShowBonesNames->setText(tr("Show bones names"));
        connect(mCheckShowBonesNames, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowBonesNamesChecked);

        vbar->AddWidget(chkShowBones);
        vbar->AddWidget(mCheckShowBonesLinks);
        vbar->AddWidget(mCheckShowBonesNames);
        mGroup3DViewSkeleton->AddWidget(vbar);
    }
    // Model
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        mCheckShowModel = new QCheckBox();
        mCheckShowModel->setText(tr("Show model"));
        mCheckShowModel->setChecked(true);
        connect(mCheckShowModel, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowModelChecked);

        QLabel* labelLOD = new QLabel();
        labelLOD->setText(tr("LOD to show:"));

        mModelLod = new QSpinBox();
        mModelLod->setMinimum(0);
        mModelLod->setMaximum(0);
        connect(mModelLod, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainRibbon::On3DViewModelLODValueChanged);

        vbar->AddWidget(mCheckShowModel);
        vbar->AddWidget(labelLOD);
        vbar->AddWidget(mModelLod);
        mGroup3DViewModel->AddWidget(vbar);
    }
    // Physics
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        mCheckShowPhysics = new QCheckBox();
        mCheckShowPhysics->setText(tr("Show physics"));
        connect(mCheckShowPhysics, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewShowPhysicsChecked);

        vbar->AddWidget(mCheckShowPhysics);
        mGroup3DViewPhysics->AddWidget(vbar);
    }
    // Renderer
    {
        SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
        QLabel* label = new QLabel();
        label->setText(tr("Renderer type:"));

        mRendererType = new QComboBox();
        for (const char* s : sRendererTypesNames) {
            mRendererType->addItem(QString(s));
        }
        mRendererType->setEditable(false);
        connect(mRendererType, &QComboBox::currentIndexChanged, this, &MainRibbon::On3DViewRendererTypeChanged);

        vbar->AddWidget(label);
        vbar->AddWidget(mRendererType);
        mGroup3DViewRenderer->AddWidget(vbar);
    }
}
