using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MetroMEControls {
    public abstract class MainWindowListener {
        public abstract void OnWindowLoaded(MainWindow wnd);
        public abstract void OnUpdate();
        public abstract void OnRenderPanelResized();
        public abstract void OnRenderPanelMouseButton(bool left, bool right, float x, float y);
        public abstract void OnRenderPanelMouseMove(float x, float y);
        public abstract void OnRenderPanelMouseWheel(float delta);
        public abstract void OnTreeViewSelectionChanged(int selectedTag);
        public abstract void OnModelPropChanged(string propName, string propValue);
        public abstract void OnFileImportMetroModelCommand(string filePath);
        public abstract void OnFileImportOBJModelCommand(string filePath);
        public abstract void OnFileExportMetroModelCommand(string filePath);
        public abstract void OnFileExportOBJModelCommand(string filePath);
        public abstract void OnFileExportFBXModelCommand(string filePath);

        // debug
        public abstract void OnDebugShowBounds(bool show);
        public abstract void OnDebugShowSubmodelsBounds(bool show);
        public abstract void OnDebugBoundsTypeChanged(string newType);
        public abstract void OnDebugSkeletonShowBones(bool show);
        public abstract void OnDebugSkeletonShowBonesLinks(bool show);
        public abstract void OnDebugSkeletonShowBonesNames(bool show);

        public abstract bool CanExportModel();
    }
}
