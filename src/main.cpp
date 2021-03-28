#include "src/ui/mainwindow.h"

#include <QApplication>

#include "mycommon.h"
#include "mex_settings.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    fs::path folder = a.applicationDirPath().toStdWString();
    LogOpen(folder);

    MEXSettings::Get().SetFolder(folder);
    if (!MEXSettings::Get().Load()) {
        LogPrint(LogLevel::Error, "Failed to load settings, initializing to defauls");
        MEXSettings::Get().InitDefaults();
    }

    MainWindow w;
    w.show();
    const int execResult = a.exec();

    MEXSettings::Get().Save();
    LogClose();

    return execResult;
}
