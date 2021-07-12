#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMimeData>
#include <QDragEnterEvent>

#include "../metropackunpack.h"

static const QString kLastOpenPath("LastOpenPath");
static const QString kLastSavePath("LastSavePath");

static bool IsPathMetroPackFile(const fs::path& filePath) {
    if (!OSPathExists(filePath) || !OSPathIsFile(filePath)) {
        return false;
    }

    WideString ext = filePath.extension().wstring();
    return (WStrStartsWith(ext, L".vfi") || WStrStartsWith(ext, L".upk") || WStrStartsWith(ext, L".vfx"));
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mProgressDlg(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    if (mThread.joinable()) {
        mThread.join();
    }

    delete ui;
}


void MainWindow::ThreadedExtractionMethod(fs::path archivePath, fs::path outputFolderPath) {
    IProgressDialog* progressDlg = mProgressDlg;

    auto progressCallback = [progressDlg](float f)->bool {
        const DWORD value = scast<DWORD>(f * 1000.0f);
        progressDlg->SetProgress(value, 1000u);

        if (progressDlg->HasUserCancelled() == TRUE) {
            return false;
        } else {
            return true;
        }
    };

    MetroPackUnpack::UnpackArchive(archivePath, outputFolderPath, progressCallback);

    if (mProgressDlg) {
        mProgressDlg->StopProgressDialog();
        MySafeRelease(mProgressDlg);
    }
}

void MainWindow::OnMetroPackSelected(const fs::path& archivePath) {
    if (!IsPathMetroPackFile(archivePath)) {
        QMessageBox::critical(this, this->windowTitle(), tr("Invalid Metro archive selected!"));
    } else {
        QString name = QFileDialog::getExistingDirectory(this, tr("Choose output folder..."));
        if (!name.isEmpty()) {
            HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IProgressDialog), (void**)&mProgressDlg);
            if (SUCCEEDED(hr)) {
                mProgressDlg->SetTitle(L"Extracting files...");
                mProgressDlg->SetLine(0, L"Please wait while your files are being extracted...", FALSE, nullptr);
                mProgressDlg->StartProgressDialog(rcast<HWND>(this->winId()), nullptr,
                    PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE,
                    nullptr);
            }

            if (mThread.joinable()) {
                mThread.join();
            }

            fs::path outputFolder = name.toStdWString();
            mThread = std::thread(&MainWindow::ThreadedExtractionMethod, this, archivePath, outputFolder);
        }
    }
}

void MainWindow::ThreadedPack2033Method(fs::path contentPath, fs::path archivePath) {
    IProgressDialog* progressDlg = mProgressDlg;

    auto progressCallback = [progressDlg](float f)->bool {
        const DWORD value = scast<DWORD>(f * 1000.0f);
        progressDlg->SetProgress(value, 1000u);

        if (progressDlg->HasUserCancelled() == TRUE) {
            return false;
        } else {
            return true;
        }
    };

    MetroPackUnpack::PackArchive2033(contentPath, archivePath, progressCallback);

    if (mProgressDlg) {
        mProgressDlg->StopProgressDialog();
        MySafeRelease(mProgressDlg);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto& urls = event->mimeData()->urls();
        if (urls.size() == 1) {
            fs::path filePath = urls.first().toLocalFile().toStdWString();
            if (IsPathMetroPackFile(filePath)) {
                event->acceptProposedAction();
            }
        }
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event) {
    event->accept();
}

void MainWindow::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto& urls = event->mimeData()->urls();
        if (urls.size() == 1) {
            fs::path filePath = urls.first().toLocalFile().toStdWString();
            if (IsPathMetroPackFile(filePath)) {
                event->acceptProposedAction();

                this->OnMetroPackSelected(filePath);
            }
        }
    }
}


void MainWindow::on_btnUnpack_clicked() {
    QString name = QFileDialog::getOpenFileName(this, tr("Open Metro archive..."), QString(), tr("Metro archive file (*.*)"));
    if (!name.isEmpty()) {
        fs::path filePath = name.toStdWString();
        this->OnMetroPackSelected(filePath);
    }
}

void MainWindow::on_btnPack2033_clicked() {
    QString name = QFileDialog::getExistingDirectory(this, tr("Choose content folder..."));
    if (!name.isEmpty()) {
        fs::path contentPath = name.toStdWString();
        if (contentPath.stem() != L"content") {
            QMessageBox::critical(this, this->windowTitle(), tr("Wrong folder! You myst select content folder!"));
        } else {
            name = QFileDialog::getSaveFileName(this, tr("Choose output archive name..."), "content.upk9");
            if (!name.isEmpty()) {
                fs::path archivePath = name.toStdWString();

                HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IProgressDialog), (void**)&mProgressDlg);
                if (SUCCEEDED(hr)) {
                    mProgressDlg->SetTitle(L"Extracting files...");
                    mProgressDlg->SetLine(0, L"Please wait while your files are being extracted...", FALSE, nullptr);
                    mProgressDlg->StartProgressDialog(rcast<HWND>(this->winId()), nullptr,
                        PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE,
                        nullptr);
                }

                if (mThread.joinable()) {
                    mThread.join();
                }

                mThread = std::thread(&MainWindow::ThreadedPack2033Method, this, contentPath, archivePath);
            }
        }
    }
}

void MainWindow::on_btnPackLastLight_clicked() {
}

void MainWindow::on_btnPackRedux_clicked() {
}

void MainWindow::on_btnPackExodus_clicked() {
}
