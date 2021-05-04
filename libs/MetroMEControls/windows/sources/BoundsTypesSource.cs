using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows;
using System.Windows.Media;

namespace MetroMEControls {
    public class BoundsTypeUI : INotifyPropertyChanged {
        private DrawingImage    mIcon;
        private string          mName;

        public BoundsTypeUI(DrawingImage icon, string name) {
            this.mIcon = icon;
            this.mName = name;
        }

        public DrawingImage Icon {
            get {
                return mIcon;
            }
            set {
                mIcon = value;
                NotifyPropertyChanged("Icon");
            }
        }

        public string Name {
            get {
                return mName;
            }
            set {
                mName = value;
                NotifyPropertyChanged("Name");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(String info) {
            if (PropertyChanged != null) {
                PropertyChanged(this, new PropertyChangedEventArgs(info));
            }
        }
    }

    public class BoundsTypesSource : ObservableCollection<BoundsTypeUI> {
        public BoundsTypesSource() {
            Add(Application.Current.FindResource("Icon_BBox") as DrawingImage, "Box");
            Add(Application.Current.FindResource("Icon_BSphere") as DrawingImage, "Sphere");

            this.SelectedItem = this[0];
        }

        public BoundsTypeUI Add(DrawingImage icon, string name) {
            BoundsTypeUI btype = new BoundsTypeUI(icon, name);
            this.Add(btype);
            return btype;
        }

        public BoundsTypeUI SelectedItem {
            get; set;
        }
    }
}
