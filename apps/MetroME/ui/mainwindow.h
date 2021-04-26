#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SimpleRibbon;

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

public slots:
    void    OnImportMetroModel();
    void    OnImportOBJModel();
    void    OnExportMetroModel();
    void    OnExportOBJModel();
    void    OnExportFBXModel();
    //
    void    OnShowBounds(bool checked);
    void    OnBoundsTypeChanged(int index);
    void    OnSubmodelBounds(bool checked);
    void    OnSkeletonShowBones(bool checked);
    void    OnSkeletonShowBonesLinks(bool checked);
    void    OnSkeletonShowBonesNames(bool checked);

private:
    Ui::MainWindow* ui;
    RenderPanel*    mRenderPanel;
};
#endif // MAINWINDOW_H
