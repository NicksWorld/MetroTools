#include "ui/MainWindowImpl.h"

//ref class MyApplication : public System::Windows::Application {
//public:
//    MyApplication() : System::Windows::Application() {
//        this->LoadCompleted += gcnew System::Windows::Navigation::LoadCompletedEventHandler(this, &MyApplication::OnLoadCompleted);
//    }
//
//protected:
//    void OnLoadCompleted(System::Object ^sender, System::Windows::Navigation::NavigationEventArgs ^e) {
//        MetroMEControls::MainWindow^ wnd = safe_cast<MetroMEControls::MainWindow^>(this->MainWindow);
//        if (wnd) {
//            wnd->SetListener(gcnew MainWindowListener());
//        }
//    }
//};



[System::STAThreadAttribute]
void Main(array<System::String^>^ args) {
    System::Windows::Application app;
    //app.StartupUri = gcnew System::Uri("/MetroMEControls;component/windows/MainWindow.xaml", System::UriKind::Relative);

    System::Windows::ResourceDictionary resDict;
    resDict.Source = gcnew System::Uri("/MetroMEControls;component/IconsResDict.xaml", System::UriKind::Relative);
    app.Resources->MergedDictionaries->Add(%resDict);

    MetroMEControls::MainWindow mainWindow;
    mainWindow.SetListener(gcnew MainWindowImpl());
    app.Run(%mainWindow);
}
