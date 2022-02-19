#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

#include "mycommon.h"


class SimpleRibbon;
class SimpleRibbonTab;
class SimpleRibbonGroup;
class SimpleRibbonVBar;
class SimpleRibbonButton;

class MainRibbon final : public QWidget {
    Q_OBJECT

public:
    enum class TabType {
        Model,
        Skeleton,
        Animation,
        Physics,
        View
    };

    enum class RendererType {
        Regular,
        Wireframe,
        Albedo,
        Normal,
        Gloss,
        Roughness,
        AO,

        NumRendererTypes
    };

    MainRibbon(QWidget* parent = nullptr);
    ~MainRibbon() override;

    void    EnableTab(const TabType tab, const bool enable);
    void    SetLODLimit(const int limit);
    void    SetPresets(const StringArray& presets);

signals:
    void    SignalCurrentTabChanged(const TabType tab);
    //
    void    SignalFileImportMetroModel();
    void    SignalFileImportOBJModel();
    void    SignalFileImportFBXModel();
    void    SignalFileExportMetroModel();
    void    SignalFileExportOBJModel();
    void    SignalFileExportFBXModel();
    void    SignalFileExportGLTFModel();
    //
    void    SignalFileImportMetroSkeleton();
    void    SignalFileImportFBXSkeleton();
    void    SignalFileExportMetroSkeleton();
    void    SignalFileExportFBXSkeleton();
    //
    void    SignalModelTPresetChanged(int index);
    void    SignalModelTPresetEditClicked();
    void    SignalModelCalculateAOClicked(int quality);
    void    SignalModelBuildLODsClicked();
    //
    void    SignalSkeletonBuildBonesOBBsClicked();
    //
    void    SignalPhysicsBuildClicked(int physicsSource);
    //
    void    Signal3DViewShowBoundsChecked(bool checked);
    void    Signal3DViewBoundsTypeChanged(int index);
    void    Signal3DViewSubmodelsBoundsChecked(bool checked);
    void    Signal3DViewShowBonesChecked(bool checked);
    void    Signal3DViewShowBonesLinksChecked(bool checked);
    void    Signal3DViewShowBonesNamesChecked(bool checked);
    void    Signal3DViewShowModelChecked(bool checked);
    void    Signal3DViewModelLODValueChanged(int value);
    void    Signal3DViewShowPhysicsChecked(bool checked);
    void    Signal3DViewRendererTypeChanged(RendererType type);

private slots:
    void    OnCurrentTabChanged(int index);
    //
    void    OnFileImportMetroModelCommand(bool checked);
    void    OnFileImportOBJModelCommand(bool checked);
    void    OnFileImportFBXModelCommand(bool checked);
    void    OnFileExportMetroModelCommand(bool checked);
    void    OnFileExportOBJModelCommand(bool checked);
    void    OnFileExportFBXModelCommand(bool checked);
    void    OnFileExportGLTFModelCommand(bool checked);
    //
    void    OnFileImportMetroSkeletonCommand(bool checked);
    void    OnFileImportFBXSkeletonCommand(bool checked);
    void    OnFileExportMetroSkeletonCommand(bool checked);
    void    OnFileExportFBXSkeletonCommand(bool checked);
    //
    void    OnModelTPresetChanged(int index);
    void    OnModelTPresetEditClicked();
    void    OnModelCalculateAOClicked();
    void    OnModelBuildLODsClicked();
    //
    void    OnSkeletonBuildBonesOBBsClicked();
    //
    void    OnPhysicsBuildButtonClicked();
    //
    void    On3DViewShowBoundsChecked(int state);
    void    On3DViewBoundsTypeChanged(int index);
    void    On3DViewSubmodelsBoundsChecked(int state);
    void    On3DViewShowBonesChecked(int state);
    void    On3DViewShowBonesLinksChecked(int state);
    void    On3DViewShowBonesNamesChecked(int state);
    void    On3DViewShowModelChecked(int state);
    void    On3DViewModelLODValueChanged(int value);
    void    On3DViewShowPhysicsChecked(int state);
    void    On3DViewRendererTypeChanged(int index);

private:
    void    BuildRibbon();
    void    BuildModelTab();
    void    BuildSkeletonTab();
    void    BuildAnimationTab();
    void    BuildPhysicsTab();
    void    Build3DViewTab();

private:
    SimpleRibbon*       mRibbon;
    // tabs
    SimpleRibbonTab*    mTabModel;
    SimpleRibbonTab*    mTabSkeleton;
    SimpleRibbonTab*    mTabAnimation;
    SimpleRibbonTab*    mTabPhysics;
    SimpleRibbonTab*    mTab3DView;
    // model groups
    SimpleRibbonGroup*  mGroupModelFile;
    SimpleRibbonGroup*  mGroupModelPreset;
    SimpleRibbonGroup*  mGroupModelAO;
    SimpleRibbonGroup*  mGroupModelLODs;
    // skeleton groups
    SimpleRibbonGroup*  mGroupSkeletonFile;
    SimpleRibbonGroup*  mGroupSkeletonOBBs;
    // animation groups
    // physics groups
    SimpleRibbonGroup*  mGroupPhysicsTools;
    // 3d view groups
    SimpleRibbonGroup*  mGroup3DViewBounds;
    SimpleRibbonGroup*  mGroup3DViewSkeleton;
    SimpleRibbonGroup*  mGroup3DViewModel;
    SimpleRibbonGroup*  mGroup3DViewPhysics;
    SimpleRibbonGroup*  mGroup3DViewRenderer;
    // model controls
    QComboBox*          mComboTPreset;
    QComboBox*          mAOCalcQuality;
    QPushButton*        mCalculateAOButton;
    QPushButton*        mBuildLODsButton;
    // skeleton controls
    QPushButton*        mBuildBonesOBBsButton;
    // physics controls
    QComboBox*          mComboPhysicsSource;
    QPushButton*        mBuildPhysicsButton;
    // 3d view controls
    QComboBox*          mComboBoundsType;
    QCheckBox*          mCheckSubmodelBounds;
    QCheckBox*          mCheckShowBonesLinks;
    QCheckBox*          mCheckShowBonesNames;
    QCheckBox*          mCheckShowPhysics;
    QCheckBox*          mCheckShowModel;
    QSpinBox*           mModelLod;
    QComboBox*          mRendererType;
};
