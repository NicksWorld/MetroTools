using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MetroMEControls {
    /// <summary>
    /// Follow steps 1a or 1b and then 2 to use this custom control in a XAML file.
    ///
    /// Step 1a) Using this custom control in a XAML file that exists in the current project.
    /// Add this XmlNamespace attribute to the root element of the markup file where it is 
    /// to be used:
    ///
    ///     xmlns:MyNamespace="clr-namespace:MetroMEControls"
    ///
    ///
    /// Step 1b) Using this custom control in a XAML file that exists in a different project.
    /// Add this XmlNamespace attribute to the root element of the markup file where it is 
    /// to be used:
    ///
    ///     xmlns:MyNamespace="clr-namespace:MetroMEControls;assembly=MetroMEControls"
    ///
    /// You will also need to add a project reference from the project where the XAML file lives
    /// to this project and Rebuild to avoid compilation errors:
    ///
    ///     Right click on the target project in the Solution Explorer and
    ///     "Add Reference"->"Projects"->[Browse to and select this project]
    ///
    ///
    /// Step 2)
    /// Go ahead and use your control in the XAML file.
    ///
    ///     <MyNamespace:RenderPanel/>
    ///
    /// </summary>
    public class RenderPanel : Border {
        public delegate void OnMouseButtonDelegate(bool left, bool right, float x, float y);
        public delegate void OnMouseMoveDelegate(float x, float y);
        public delegate void OnMouseWheelDelegate(float delta);

        public event OnMouseButtonDelegate OnMouseButtonEvent = null;
        public event OnMouseMoveDelegate OnMouseMoveEvent = null;
        public event OnMouseWheelDelegate OnMouseWheelEvent = null;

        private HwndControl mHwndControl = null;

        static RenderPanel() {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(RenderPanel), new FrameworkPropertyMetadata(typeof(RenderPanel)));
        }

        public RenderPanel() {
        }

        public void PrepareHwnd() {
            this.SizeChanged += RenderPanel_SizeChanged;

            Size size = WPFSizeConvertion.GetElementPixelSize(this);

            mHwndControl = new HwndControl(size.Width, size.Height);
            this.Child = mHwndControl;
            mHwndControl.MessageHook += MHwndControl_MessageHook;

            var tme = new WinApi.TRACKMOUSEEVENT(WinApi.TMEFlags.TME_LEAVE, mHwndControl.Hwnd, 0);
            WinApi.TrackMouseEvent(ref tme);
        }

        private IntPtr MHwndControl_MessageHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled) {
            handled = false;

            switch (msg) {
                case WinApi.WM_NCHITTEST: {
                    handled = true;
                    return new IntPtr(WinApi.HTCLIENT);
                }

                case WinApi.WM_IME_SETCONTEXT: {
                    if (WinApi.LOWORD(wParam) > 0) {
                        WinApi.SetFocus(mHwndControl.Hwnd);
                    }

                    handled = true;
                } break;

                case WinApi.WM_MOUSEMOVE: {
                    float xPos = (float)WinApi.LOWORD(lParam);
                    float yPos = (float)WinApi.HIWORD(lParam);

                    this.OnMouseMoveEvent?.Invoke(xPos, yPos);

                    handled = true;
                } break;

                case WinApi.WM_LBUTTONDOWN:
                case WinApi.WM_LBUTTONUP: {
                    bool left = (0 != ((long)wParam & 0x0001)); // MK_LBUTTON

                    float xPos = (float)WinApi.LOWORD(lParam);
                    float yPos = (float)WinApi.HIWORD(lParam);

                    this.OnMouseButtonEvent?.Invoke(left, false, xPos, yPos);

                    handled = true;
                } break;

                case WinApi.WM_MOUSEWHEEL: {
                    int delta = unchecked(WinApi.GET_WHEEL_DELTA_WPARAM(wParam));

                    float actualDelta = (float)delta / 120.0f;

                    this.OnMouseWheelEvent?.Invoke(actualDelta);

                    handled = true;
                } break;

                case WinApi.WM_MOUSELEAVE: {
                    this.OnMouseButtonEvent?.Invoke(false, false, 0.0f, 0.0f);

                    handled = true;
                } break;
            }

            return IntPtr.Zero;
        }

        private void RenderPanel_SizeChanged(object sender, SizeChangedEventArgs e) {
            Size size = WPFSizeConvertion.GetElementPixelSize(this);
            mHwndControl?.OnChangeSize(size.Width, size.Height);
        }

        public IntPtr GetHwnd() {
            return (mHwndControl == null) ? IntPtr.Zero : mHwndControl.Hwnd;
        }
    }
}
