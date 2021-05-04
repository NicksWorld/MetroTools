using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Microsoft.Win32;

namespace MetroMEControls {
    //public class MyItem : INotifyPropertyChanged {
    //    private string mName = string.Empty;
    //    private string mValue = string.Empty;

    //    public MyItem(string name, string value) {
    //        this.mName = name;
    //        this.mValue = value;
    //    }

    //    public string Name {
    //        get {
    //            return this.mName;
    //        }
    //        set {
    //            this.mName = value;
    //            NotifyPropertyChanged("Name");
    //        }
    //    }

    //    public string Value {
    //        get {
    //            return this.mValue;
    //        }
    //        set {
    //            this.mValue = value;
    //            NotifyPropertyChanged("Value");
    //        }
    //    }

    //    public event PropertyChangedEventHandler PropertyChanged;
    //    private void NotifyPropertyChanged(String info) {
    //        if (PropertyChanged != null) {
    //            PropertyChanged(this, new PropertyChangedEventArgs(info));
    //        }
    //    }
    //}

    public class MyItem {
        public string Name { get; set; }
        public string Value { get; set; }

        public MyItem(string name, string value) {
            this.Name = name;
            this.Value = value;
        }
    }


    public partial class MainWindow : Window {
        BoundsTypesSource mBoundsTypesSource = new BoundsTypesSource();
        List<MyItem> mModelPropsSource = new List<MyItem>();
        MainWindowListener mListener = null;

        public MainWindow() {
            InitializeComponent();
        }

        public void SetListener(MainWindowListener listener) {
            mListener = listener;
        }

        public IntPtr GetRenderPanelHwnd() {
            return this.renderPanel.GetHwnd();
        }

        public int GetRenderPanelWidth() {
            return (int)WPFSizeConvertion.GetElementPixelSize(this.renderPanel).Width;
        }

        public int GetRenderPanelHeight() {
            return (int)WPFSizeConvertion.GetElementPixelSize(this.renderPanel).Height;
        }

        public void EnableSkeletonTab(bool b) {
            this.ribbonTabSkeleton.IsEnabled = b;
        }

        public void TreeViewReset(string rootText, int rootTag) {
            this.modelTreeView.Items.Clear();

            if (rootText.Length > 0) {
                this.modelTreeView.Items.Add(new TreeViewItem { Header = rootText, IsExpanded = true, Tag = rootTag });
            }
        }

        public void TreeViewAddSub(string text, int tag) {
            if (this.modelTreeView.Items.Count > 0) {
                TreeViewItem root = this.modelTreeView.Items[0] as TreeViewItem;
                if (root != null) {
                    root.Items.Add(new TreeViewItem { Header = text, Tag = tag });
                }
            }
        }

        public void ModelPropsSetArray(List<Tuple<string, string>> props) {
            mModelPropsSource.Clear();

            if (props != null) {
                foreach (var i in props) {
                    mModelPropsSource.Add(new MyItem(i.Item1, i.Item2));
                }
            }

            this.lstModelProps.ItemsSource = mModelPropsSource;
            this.lstModelProps.Items.Refresh();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e) {
            this.ribbonDebugBoundsTypeCombobox.DataContext = mBoundsTypesSource;
            this.lstModelProps.ItemsSource = mModelPropsSource;

            this.renderPanel.PrepareHwnd();

            mListener?.OnWindowLoaded(this);

            this.renderPanel.SizeChanged += RenderPanel_SizeChanged;
            this.renderPanel.OnMouseButtonEvent += RenderPanel_OnMouseButtonEvent;
            this.renderPanel.OnMouseMoveEvent += RenderPanel_OnMouseMoveEvent;
            this.renderPanel.OnMouseWheelEvent += RenderPanel_OnMouseWheelEvent;

            DispatcherTimer timer = new DispatcherTimer(DispatcherPriority.Normal);
            timer.Tick += Timer_Tick;
            timer.Interval = new TimeSpan(0, 0, 0, 0, 16);
            timer.Start();
        }

        private void RenderPanel_OnMouseWheelEvent(float delta) {
            mListener?.OnRenderPanelMouseWheel(delta);
        }

        private void RenderPanel_OnMouseMoveEvent(float x, float y) {
            mListener?.OnRenderPanelMouseMove(x, y);
        }

        private void RenderPanel_OnMouseButtonEvent(bool left, bool right, float x, float y) {
            System.Windows.Input.Keyboard.ClearFocus();

            mListener?.OnRenderPanelMouseButton(left, right, x, y);
        }

        private void RenderPanel_SizeChanged(object sender, SizeChangedEventArgs e) {
            mListener?.OnRenderPanelResized();
        }

        private void Timer_Tick(object sender, EventArgs e) {
            mListener?.OnUpdate();
        }

        private void BtnCollapseRibbon_Click(object sender, RoutedEventArgs e) {
            this.ribbonMain.IsMinimized = true;
            this.btnCollapseRibbon.Visibility = Visibility.Collapsed;
            this.btnExpandRibbon.Visibility = Visibility.Visible;
        }

        private void BtnExpandRibbon_Click(object sender, RoutedEventArgs e) {
            this.ribbonMain.IsMinimized = false;
            this.btnCollapseRibbon.Visibility = Visibility.Visible;
            this.btnExpandRibbon.Visibility = Visibility.Collapsed;
        }

        private void RibbonAppMenu_Exit_Click(object sender, RoutedEventArgs e) {
            this.Close();
        }

        private void MenuRibbonModelFileImportMetroModel_Click(object sender, RoutedEventArgs e) {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Metro model files (*.model)|*.model|Metro mesh files (*.mesh)|*.mesh|All files (*.*)|*.*";
            ofd.Title = "Choose Metro model/mesh file...";
            ofd.RestoreDirectory = true;
            if (ofd.ShowDialog(this) == true) {
                mListener?.OnFileImportMetroModelCommand(ofd.FileName);
            }
        }

        private void MenuRibbonModelFileImportOBJModel_Click(object sender, RoutedEventArgs e) {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "OBJ model files (*.obj)|*.obj|All files (*.*)|*.*";
            ofd.Title = "Choose OBJ model file...";
            ofd.RestoreDirectory = true;
            if (ofd.ShowDialog(this) == true) {
                mListener?.OnFileImportOBJModelCommand(ofd.FileName);
            }
        }

        private void MenuRibbonModelFileExportMetroModel_Click(object sender, RoutedEventArgs e) {
            if (mListener != null && mListener.CanExportModel()) {
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.Filter = "Metro Model file (*.model)|*.model|All files (*.*)|*.*";
                sfd.Title = "Where to export Metro model...";
                sfd.RestoreDirectory = true;
                if (sfd.ShowDialog(this) == true) {
                    mListener?.OnFileExportMetroModelCommand(sfd.FileName);
                }
            }
        }

        private void MenuRibbonModelFileExportOBJModel_Click(object sender, RoutedEventArgs e) {
            if (mListener != null && mListener.CanExportModel()) {
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.Filter = "OBJ model file (*.obj)|*.obj|All files (*.*)|*.*";
                sfd.Title = "Where to export OBJ model...";
                sfd.RestoreDirectory = true;
                if (sfd.ShowDialog(this) == true) {
                    mListener?.OnFileExportOBJModelCommand(sfd.FileName);
                }
            }
        }

        private void MenuRibbonModelFileExportFBXModel_Click(object sender, RoutedEventArgs e) {
            if (mListener != null && mListener.CanExportModel()) {
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.Filter = "FBX model file (*.fbx)|*.fbx|All files (*.*)|*.*";
                sfd.Title = "Where to export FBX model...";
                sfd.RestoreDirectory = true;
                if (sfd.ShowDialog(this) == true) {
                    mListener?.OnFileExportFBXModelCommand(sfd.FileName);
                }
            }
        }

        private void BtnRibbonHelp_Click(object sender, RoutedEventArgs e) {
            AboutWnd wnd = new AboutWnd();
            wnd.Owner = this;
            wnd.ShowDialog();
        }

        private void ModelTreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e) {
            var tvi = this.modelTreeView.SelectedItem as TreeViewItem;
            if (tvi != null) {
                mListener?.OnTreeViewSelectionChanged((int)tvi.Tag);
            }
        }

        private void RibbonDebugShowBoundsCheckbox_Click(object sender, RoutedEventArgs e) {
            mListener?.OnDebugShowBounds(this.ribbonDebugShowBoundsCheckbox.IsChecked == true);
        }

        private void RibbonDebugShowSubmodelsBoundsCheckbox_Click(object sender, RoutedEventArgs e) {
            mListener?.OnDebugShowSubmodelsBounds(this.ribbonDebugShowSubmodelsBoundsCheckbox.IsChecked == true);
        }

        private void RibbonDebugBoundsTypeComboboxGallery_SelectionChanged(object sender, RoutedPropertyChangedEventArgs<object> e) {
            var selected = this.ribbonDebugBoundsTypeComboboxGallery.SelectedItem as BoundsTypeUI;
            if (selected != null) {
                mListener?.OnDebugBoundsTypeChanged(selected.Name);
            }
        }

        private void RibbonDebugShowBones_Click(object sender, RoutedEventArgs e) {
            mListener?.OnDebugSkeletonShowBones(this.ribbonDebugShowBones.IsChecked == true);
        }

        private void RibbonDebugShowBonesLinks_Click(object sender, RoutedEventArgs e) {
            mListener?.OnDebugSkeletonShowBonesLinks(this.ribbonDebugShowBonesLinks.IsChecked == true);
        }

        private void RibbonDebugShowBonesNames_Click(object sender, RoutedEventArgs e) {
            mListener?.OnDebugSkeletonShowBonesNames(this.ribbonDebugShowBonesNames.IsChecked == true);
        }

        private void ModelPropTextBox_TextChanged(object sender, TextChangedEventArgs e) {
            var txtBox = sender as TextBox;
            var item = txtBox.DataContext as MyItem;
            if (item != null) {
                mListener?.OnModelPropChanged(item.Name, txtBox.Text);
            }
        }
    }
}
