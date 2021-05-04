using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;

namespace MetroMEControls {
    public class WPFSizeConvertion {
        public static Size GetElementPixelSize(FrameworkElement element) {
            Matrix transformToDevice;
            var source = PresentationSource.FromVisual(element);
            if (source != null) {
                transformToDevice = source.CompositionTarget.TransformToDevice;
            } else {
                using (var hwndSource = new HwndSource(new HwndSourceParameters())) {
                    transformToDevice = hwndSource.CompositionTarget.TransformToDevice;
                }
            }

            //if (element.DesiredSize == new Size()) {
            //    element.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
            //}

            //return (Size)transformToDevice.Transform((Vector)element.DesiredSize);
            //return (Size)transformToDevice.Transform((Vector)element.RenderSize);

            Vector size = new Vector(element.ActualWidth, element.ActualHeight);
            return (Size)transformToDevice.Transform(size);
        }
    }
}
