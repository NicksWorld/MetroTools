using System;
using System.Runtime.InteropServices;
using System.Windows.Input;
using System.Windows.Interop;

namespace MetroMEControls {
    public class HwndControl : HwndHost {
        internal const int
            WsChild             = 0x40000000,
            WsVisible           = 0x10000000,
            HostId              = 0x00000002,
            SWP_NOACTIVATE      = 0x0010,
            SWP_NOMOVE          = 0x0002,
            SWP_NOZORDER        = 0x0004;

        private int mHostHeight;
        private int mHostWidth;
        private IntPtr mHwndHost;

        public HwndControl(double width, double height) {
            mHostWidth = (int)width;
            mHostHeight = (int)height;
        }

        public IntPtr Hwnd {
            get {
                return mHwndHost;
            }
        }

        public void OnChangeSize(double width, double height) {
            int newWidth = (int)width;
            int newHeight = (int)height;

            if (newWidth != mHostWidth || newHeight != mHostHeight) {
                mHostWidth = newWidth;
                mHostHeight = newHeight;
                SetWindowPos(this.Hwnd, IntPtr.Zero, 0, 0, newWidth, newHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
            }
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent) {
            mHwndHost = IntPtr.Zero;

            mHwndHost = CreateWindowEx(0, "static", "",
                WsChild | WsVisible,
                0, 0,
                mHostHeight, mHostWidth,
                hwndParent.Handle,
                (IntPtr)HostId,
                IntPtr.Zero,
                0);

            return new HandleRef(this, mHwndHost);
        }

        protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled) {
            handled = false;
            return IntPtr.Zero;
        }

        protected override void DestroyWindowCore(HandleRef hwnd) {
            DestroyWindow(hwnd.Handle);
        }

        //PInvoke declarations
        [DllImport("user32.dll", EntryPoint = "CreateWindowEx", CharSet = CharSet.Unicode)]
        internal static extern IntPtr CreateWindowEx(int dwExStyle,
            string lpszClassName,
            string lpszWindowName,
            int style,
            int x, int y,
            int width, int height,
            IntPtr hwndParent,
            IntPtr hMenu,
            IntPtr hInst,
            [MarshalAs(UnmanagedType.AsAny)] object pvParam);

        [DllImport("user32.dll", EntryPoint = "DestroyWindow", CharSet = CharSet.Unicode)]
        internal static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll", EntryPoint = "SetWindowPos", CharSet = CharSet.Unicode)]
        internal static extern int SetWindowPos(IntPtr hwnd,
            IntPtr hWndInsertAfter,
            int x,
            int y,
            int cx,
            int cy,
            uint flags);
    }
}
