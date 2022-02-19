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
class MetroPhysicsCForm;


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
    void    UpdatePhysicsFromTheModel(MetroModelBase* model, const fs::path& modelPath);
    void    UpdatePhysicsFromCForm(RefPtr<MetroPhysicsCForm>& cform);

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
    void    OnTPresetChanged(int index);
    void    OnTPresetsEdit();
    void    OnCalculateAO(int quality);
    void    OnBuildLODs();
    //
    void    OnPhysicsBuild(int physicsSource);
    //
    void    OnSkeletonBuildOBBs();
    //
    void    OnShowBounds(bool checked);
    void    OnBoundsTypeChanged(int index);
    void    OnSubmodelBounds(bool checked);
    void    OnSkeletonShowBones(bool checked);
    void    OnSkeletonShowBonesLinks(bool checked);
    void    OnSkeletonShowBonesNames(bool checked);
    void    OnShowModel(bool checked);
    void    OnModelLODValueChanged(int value);
    void    OnShowPhysics(bool checked);
    void    OnRendererTypeChanged(MainRibbon::RendererType type);

private slots:
    // Model rollouts
    void    OnModelMeshSelectionChanged(int idx);
    void    OnModelMeshPropertiesChanged();

private:
    Ui::MainWindow*                 ui;
    RenderPanel*                    mRenderPanel;
    RefPtr<MetroPhysicsCForm>       mModelPhysx;
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
