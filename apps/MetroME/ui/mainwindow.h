#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "props/objectpropertybrowser.h"

#include "common/mycommon.h"

#include "mainribbon.h"


class SimpleRibbon;
class MetroModelBase;
class MaterialStringsProp;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RenderPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void UpdateUIForTheModel(MetroModelBase* model);

public slots:
    void    OnWindowLoaded();
    //
    void    OnRibbonTabChanged(const MainRibbon::TabType tab);
    //
    void    OnImportMetroModel();
    void    OnImportOBJModel();
    void    OnExportMetroModel();
    void    OnExportOBJModel();
    void    OnExportFBXModel();
    void    OnExportGLTFModel();
    //
    void    OnShowBounds(bool checked);
    void    OnBoundsTypeChanged(int index);
    void    OnSubmodelBounds(bool checked);
    void    OnSkeletonShowBones(bool checked);
    void    OnSkeletonShowBonesLinks(bool checked);
    void    OnSkeletonShowBonesNames(bool checked);
    //
    void    OnPropertyBrowserObjectPropertyChanged();

private slots:
    void    OnModelHierarchyTreeCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void    OnSkeletonHierarchyTreeCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    Ui::MainWindow*                 ui;
    RenderPanel*                    mRenderPanel;
    QTreeWidget*                    mModelHierarchyTree;
    QTreeWidget*                    mSkeletonHierarchyTree;
    ObjectPropertyBrowser*          mModelPropertyBrowser;
    ObjectPropertyBrowser*          mSkeletonPropertyBrowser;
    int                             mSelectedGD;
    StrongPtr<MaterialStringsProp>  mMatStringsProp;
    bool                            mIsInSkeletonView;
};
#endif // MAINWINDOW_H
