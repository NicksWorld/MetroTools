#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <thread>           // std::thread
#include <atomic>

#include "mycommon.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool IsProgressCancelled() const;

private:
    void ThreadedExtractionMethod(fs::path archivePath, fs::path outputFolderPath);
    void OnMetroPackSelected(const fs::path& archivePath);
    void ThreadedPack2033Method(fs::path contentPath, fs::path archivePath, const bool useCompression);

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
    void onProgressCancelled();
    void onProgressFinished();

private:
    Ui::MainWindow*     ui;
    QProgressDialog*    mProgressDlg;
    std::thread         mThread;
    std::atomic_bool    mProgressCancelled;
};

#endif // MAINWINDOW_H
