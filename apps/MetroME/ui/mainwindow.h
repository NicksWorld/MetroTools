#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Model rollouts
#include "rollouts/modelmeshesrollout.h"
#include "rollouts/modelphysxrollout.h"

// Skeleton rollouts
#include "rollouts/boneslistrollout.h"
#include "rollouts/motionsrollout.h"
#include "rollouts/facefxrollout.h"
#include "rollouts/paramsrollout.h"

#include "common/mycommon.h"

#include "mainribbon.h"


class SimpleRibbon;
class MetroModelBase;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RenderPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void    UpdateUIForTheModel(MetroModelBase* model);

public slots:
    void    OnWindowLoaded();
    //
    void    OnRibbonTabChanged(const MainRibbon::TabType tab);
    //
    void    OnImportMetroModel();
    void    OnImportOBJModel();
    void    OnImportFBXModel();
    void    OnExportMetroModel();
    void    OnExportOBJModel();
    void    OnExportFBXModel();
    void    OnExportGLTFModel();
    //
    void    OnImportMetroSkeleton();
    void    OnImportFBXSkeleton();
    void    OnExportMetroSkeleton();
    void    OnExportFBXSkeleton();
    //
    void    OnShowBounds(bool checked);
    void    OnBoundsTypeChanged(int index);
    void    OnSubmodelBounds(bool checked);
    void    OnSkeletonShowBones(bool checked);
    void    OnSkeletonShowBonesLinks(bool checked);
    void    OnSkeletonShowBonesNames(bool checked);

private slots:
    // Model rollouts
    void    OnModelMeshSelectionChanged(int idx);
    void    OnModelMeshPropertiesChanged();

private:
    Ui::MainWindow*                 ui;
    RenderPanel*                    mRenderPanel;
    // Model rollouts
    ModelMeshesRollout*             mModelMeshesRollout;
    ModelPhysXRollout*              mModelPhysXRollout;
    // Skeleton rollouts
    BonesListRollout*               mBonesListRollout;
    MotionsRollout*                 mMotionsRollout;
    FaceFXRollout*                  mFaceFXRollout;
    ParamsRollout*                  mParamsRollout;
    //
    bool                            mIsInSkeletonView;
};
#endif // MAINWINDOW_H
