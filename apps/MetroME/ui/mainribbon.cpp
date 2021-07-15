#include "mainribbon.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QSpinBox>

#include "simpleribbon/SimpleRibbon.h"
#include "simpleribbon/SimpleRibbonTab.h"
#include "simpleribbon/SimpleRibbonGroup.h"
#include "simpleribbon/SimpleRibbonVBar.h"
#include "simpleribbon/SimpleRibbonButton.h"



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
    // 3d view groups
    , mGroup3DViewBounds(nullptr)
    , mGroup3DViewSkeleton(nullptr)
    , mGroup3DViewModel(nullptr)
    // 3d view controls
    , mComboBoundsType(nullptr)
    , mCheckSubmodelBounds(nullptr)
    , mCheckShowBonesLinks(nullptr)
    , mCheckShowBonesNames(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->BuildRibbon();
}
MainRibbon::~MainRibbon() {
}

void MainRibbon::EnableSkeletonTab(const bool enable) {
    mTabSkeleton->setEnabled(enable);
}

void MainRibbon::OnFileImportMetroModelCommand(bool) {
    emit SignalFileImportMetroModel();
}
void MainRibbon::OnFileImportOBJModelCommand(bool) {
    emit SignalFileImportOBJModel();
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
    mTab3DView = mRibbon->AddRibbonTab(tr("3D View"));

    this->BuildModelTab();
    this->BuildSkeletonTab();
    this->BuildAnimationTab();
    this->Build3DViewTab();
}

void MainRibbon::BuildModelTab() {
    mGroupModelFile = mTabModel->AddRibbonGroup(tr("File"));

    SimpleRibbonButton* importModelButton = new SimpleRibbonButton;
    importModelButton->SetText(tr("Import..."));
    importModelButton->SetTooltip(tr("Import model from file"));
    importModelButton->SetIcon(QIcon(":/imgs/ImportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* importFromModelAction = menu->addAction("Import from Metro model...");
        menu->addSeparator();
        QAction* importFromObjAction = menu->addAction("Import from OBJ model...");
        importModelButton->SetMenu(menu);

        connect(importFromModelAction, &QAction::triggered, this, &MainRibbon::OnFileImportMetroModelCommand);
        connect(importFromObjAction, &QAction::triggered, this, &MainRibbon::OnFileImportOBJModelCommand);
    }
    mGroupModelFile->AddWidget(importModelButton);

    // Add 'Export' button
    SimpleRibbonButton* exportModelButton = new SimpleRibbonButton;
    exportModelButton->SetText(tr("Export..."));
    exportModelButton->SetTooltip(tr("Export model to file"));
    exportModelButton->SetIcon(QIcon(":/imgs/ExportFile.png"));
    {
        QMenu* menu = new QMenu("");
        QAction* exportToModelAction = menu->addAction("Export to Metro model...");
        menu->addSeparator();
        QAction* exportToOBJAction = menu->addAction("Export to OBJ model...");
        menu->addSeparator();
        QAction* exportToFBXAction = menu->addAction("Export to FBX model...");
        menu->addSeparator();
        QAction* exportToGLTFAction = menu->addAction("Export to GLTF model...");
        exportModelButton->SetMenu(menu);

        connect(exportToModelAction, &QAction::triggered, this, &MainRibbon::OnFileExportMetroModelCommand);
        connect(exportToOBJAction, &QAction::triggered, this, &MainRibbon::OnFileExportOBJModelCommand);
        connect(exportToFBXAction, &QAction::triggered, this, &MainRibbon::OnFileExportFBXModelCommand);
        connect(exportToGLTFAction, &QAction::triggered, this, &MainRibbon::OnFileExportGLTFModelCommand);
    }
    mGroupModelFile->AddWidget(exportModelButton);
}

void MainRibbon::BuildSkeletonTab() {
}

void MainRibbon::BuildAnimationTab() {
}

void MainRibbon::Build3DViewTab() {
    mGroup3DViewBounds = mTab3DView->AddRibbonGroup(tr("Bounds"));
    mGroup3DViewSkeleton = mTab3DView->AddRibbonGroup(tr("Skeleton"));
    mGroup3DViewModel = mTab3DView->AddRibbonGroup(tr("Model"));

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
        connect(chkShowBounds, &QCheckBox::stateChanged, this, &MainRibbon::On3DViewSubmodelsBoundsChecked);

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
        QLabel* labelLOD = new QLabel();
        labelLOD->setText(tr("LOD to show:"));

        QSpinBox* lodUpDown = new QSpinBox();
        lodUpDown->setMinimum(0);
        lodUpDown->setMaximum(0);

        vbar->AddWidget(labelLOD);
        vbar->AddWidget(lodUpDown);
        mGroup3DViewModel->AddWidget(vbar);
    }
}
