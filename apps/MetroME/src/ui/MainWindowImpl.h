#pragma once

class RenderPanel;
class MetroModelBase;

public ref class MainWindowImpl : public MetroMEControls::MainWindowListener {
public:
    MainWindowImpl();
    ~MainWindowImpl();

    virtual void OnWindowLoaded(MetroMEControls::MainWindow^ wnd) override;
    virtual void OnUpdate() override;
    virtual void OnRenderPanelResized() override;
    virtual void OnRenderPanelMouseButton(bool left, bool right, float x, float y) override;
    virtual void OnRenderPanelMouseMove(float x, float y) override;
    virtual void OnRenderPanelMouseWheel(float delta) override;
    virtual void OnTreeViewSelectionChanged(int selectedTag) override;
    virtual void OnModelPropChanged(System::String^ propName, System::String^ propValue) override;
    virtual void OnFileImportMetroModelCommand(System::String^ filePath) override;
    virtual void OnFileImportOBJModelCommand(System::String^ filePath) override;
    virtual void OnFileExportMetroModelCommand(System::String^ filePath) override;
    virtual void OnFileExportOBJModelCommand(System::String^ filePath) override;
    virtual void OnFileExportFBXModelCommand(System::String^ filePath) override;

    // debug
    virtual void OnDebugShowBounds(bool show) override;
    virtual void OnDebugShowSubmodelsBounds(bool show) override;
    virtual void OnDebugBoundsTypeChanged(System::String^ newType) override;
    virtual void OnDebugSkeletonShowBones(bool show) override;
    virtual void OnDebugSkeletonShowBonesLinks(bool show) override;
    virtual void OnDebugSkeletonShowBonesNames(bool show) override;

    virtual bool CanExportModel() override;

private:
    void UpdateUIForTheModel(MetroModelBase* model);

private:
    MetroMEControls::MainWindow^    mWindow;
    RenderPanel*                    mRenderPanel;
    int                             mSelectedGD;
};
