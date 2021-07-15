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
    MainRibbon(QWidget* parent = nullptr);
    ~MainRibbon() override;

    void    EnableSkeletonTab(const bool enable);

signals:
    void    SignalFileImportMetroModel();
    void    SignalFileImportOBJModel();
    void    SignalFileExportMetroModel();
    void    SignalFileExportOBJModel();
    void    SignalFileExportFBXModel();
    void    SignalFileExportGLTFModel();
    //
    void    Signal3DViewShowBoundsChecked(bool checked);
    void    Signal3DViewBoundsTypeChanged(int index);
    void    Signal3DViewSubmodelsBoundsChecked(bool checked);
    void    Signal3DViewShowBonesChecked(bool checked);
    void    Signal3DViewShowBonesLinksChecked(bool checked);
    void    Signal3DViewShowBonesNamesChecked(bool checked);

private slots:
    void    OnFileImportMetroModelCommand(bool checked);
    void    OnFileImportOBJModelCommand(bool checked);
    void    OnFileExportMetroModelCommand(bool checked);
    void    OnFileExportOBJModelCommand(bool checked);
    void    OnFileExportFBXModelCommand(bool checked);
    void    OnFileExportGLTFModelCommand(bool checked);
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
