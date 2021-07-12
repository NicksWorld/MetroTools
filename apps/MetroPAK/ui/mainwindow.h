#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <shlobj_core.h>    // IProgressDialog
#include <thread>           // std::thread

#include "mycommon.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void ThreadedExtractionMethod(fs::path archivePath, fs::path outputFolderPath);
    void OnMetroPackSelected(const fs::path& archivePath);
    void ThreadedPack2033Method(fs::path contentPath, fs::path archivePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void on_btnUnpack_clicked();
    void on_btnPack2033_clicked();
    void on_btnPackLastLight_clicked();
    void on_btnPackRedux_clicked();
    void on_btnPackExodus_clicked();

private:
    Ui::MainWindow*     ui;
    IProgressDialog*    mProgressDlg;
    std::thread         mThread;
};

#endif // MAINWINDOW_H
