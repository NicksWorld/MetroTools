#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>


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
        View
    };

    MainRibbon(QWidget* parent = nullptr);
    ~MainRibbon() override;

    void    EnableTab(const TabType tab, const bool enable);

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
    void    Signal3DViewShowBoundsChecked(bool checked);
    void    Signal3DViewBoundsTypeChanged(int index);
    void    Signal3DViewSubmodelsBoundsChecked(bool checked);
    void    Signal3DViewShowBonesChecked(bool checked);
    void    Signal3DViewShowBonesLinksChecked(bool checked);
    void    Signal3DViewShowBonesNamesChecked(bool checked);

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
    void    On3DViewShowBoundsChecked(int state);
    void    On3DViewBoundsTypeChanged(int index);
    void    On3DViewSubmodelsBoundsChecked(int state);
    void    On3DViewShowBonesChecked(int state);
    void    On3DViewShowBonesLinksChecked(int state);
    void    On3DViewShowBonesNamesChecked(int state);

private:
    void    BuildRibbon();
    void    BuildModelTab();
    void    BuildSkeletonTab();
    void    BuildAnimationTab();
    void    Build3DViewTab();

private:
    SimpleRibbon*       mRibbon;
    // tabs
    SimpleRibbonTab*    mTabModel;
    SimpleRibbonTab*    mTabSkeleton;
    SimpleRibbonTab*    mTabAnimation;
    SimpleRibbonTab*    mTab3DView;
    // model groups
    SimpleRibbonGroup*  mGroupModelFile;
    // skeleton groups
    SimpleRibbonGroup*  mGroupSkeletonFile;
    // 3d view groups
    SimpleRibbonGroup*  mGroup3DViewBounds;
    SimpleRibbonGroup*  mGroup3DViewSkeleton;
    SimpleRibbonGroup*  mGroup3DViewModel;
    // 3d view controls
    QComboBox*          mComboBoundsType;
    QCheckBox*          mCheckSubmodelBounds;
    QCheckBox*          mCheckShowBonesLinks;
    QCheckBox*          mCheckShowBonesNames;
};
