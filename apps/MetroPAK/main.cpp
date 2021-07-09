#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    //#NOTE_SK: this is for QSettings
    a.setOrganizationName("MetroTools");
    a.setApplicationName("MetroPAK");

    MainWindow w;
    w.show();
    return a.exec();
}
