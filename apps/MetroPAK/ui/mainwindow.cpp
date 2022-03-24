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


constexpr int kMaximumProgressValue = 10000;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mProgressDlg(nullptr)
    , mProgressCancelled(true)
{
    this->setWindowFlags(this->windowFlags() &= (~Qt::WindowMaximizeButtonHint));
    this->setWindowFlags(this->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    ui->setupUi(this);

    mProgressDlg = new QProgressDialog(this);
    mProgressDlg->close();
}

MainWindow::~MainWindow() {
    if (mThread.joinable()) {
        mThread.join();
    }

    delete ui;
    delete mProgressDlg;
}

bool MainWindow::IsProgressCancelled() const {
    return mProgressCancelled;
}


void MainWindow::ThreadedExtractionMethod(fs::path archivePath, fs::path outputFolderPath) {
    QProgressDialog* progressDlg = mProgressDlg;
    MainWindow* wnd = this;

    auto progressCallback = [progressDlg, wnd](float f)->bool {
        const int value = scast<int>(f * kMaximumProgressValue);
        QMetaObject::invokeMethod(progressDlg, "setValue", Qt::QueuedConnection, Q_ARG(int, value));

        if (wnd->IsProgressCancelled()) {
            return false;
        } else {
            return true;
        }
    };

    MetroPackUnpack::UnpackArchive(archivePath, outputFolderPath, progressCallback);

    QMetaObject::invokeMethod(this, "onProgressFinished", Qt::QueuedConnection);
}

void MainWindow::OnMetroPackSelected(const fs::path& archivePath) {
    if (!IsPathMetroPackFile(archivePath)) {
        QMessageBox::critical(this, this->windowTitle(), tr("Invalid Metro archive selected!"));
    } else {
        QString dir = QString::fromStdWString(archivePath.parent_path().wstring());

        QString name = QFileDialog::getExistingDirectory(this, tr("Choose output folder..."), dir);
        if (!name.isEmpty()) {
            this->onProgressFinished();

            mProgressDlg->setWindowTitle(tr("Extracting files..."));
            mProgressDlg->setLabelText(tr("Please wait while your files are being extracted..."));
            mProgressDlg->setMinimum(0);
            mProgressDlg->setMaximum(kMaximumProgressValue);
            mProgressDlg->setAutoClose(false);
            mProgressDlg->setWindowModality(Qt::WindowModal);
            mProgressCancelled = false;
            connect(mProgressDlg, &QProgressDialog::canceled, this, &MainWindow::onProgressCancelled);

            if (mThread.joinable()) {
                mThread.join();
            }

            fs::path outputFolder = name.toStdWString();
            mThread = std::thread(&MainWindow::ThreadedExtractionMethod, this, archivePath, outputFolder);

            mProgressDlg->show();
        }
    }
}

void MainWindow::ThreadedPack2033Method(fs::path contentPath, fs::path archivePath, const bool useCompression) {
    QProgressDialog* progressDlg = mProgressDlg;
    MainWindow* wnd = this;

    auto progressCallback = [progressDlg, wnd](float f)->bool {
        const int value = scast<int>(f * kMaximumProgressValue);
        QMetaObject::invokeMethod(progressDlg, "setValue", Qt::QueuedConnection, Q_ARG(int, value));

        if (wnd->IsProgressCancelled()) {
            return false;
        } else {
            return true;
        }
    };

    MetroPackUnpack::PackArchive2033(contentPath, archivePath, useCompression, progressCallback);

    QMetaObject::invokeMethod(this, "onProgressFinished", Qt::QueuedConnection);
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

                this->onProgressFinished();

                mProgressDlg->setWindowTitle(tr("Creating Metro 2033 archive..."));
                mProgressDlg->setLabelText(tr("Please wait while your files are being archived..."));
                mProgressDlg->setMinimum(0);
                mProgressDlg->setMaximum(kMaximumProgressValue);
                mProgressDlg->setAutoClose(false);
                mProgressDlg->setWindowModality(Qt::WindowModal);
                mProgressCancelled = false;
                connect(mProgressDlg, &QProgressDialog::canceled, this, &MainWindow::onProgressCancelled);

                if (mThread.joinable()) {
                    mThread.join();
                }

                const bool useCompression = ui->chkCompressFiles->isChecked();
                mThread = std::thread(&MainWindow::ThreadedPack2033Method, this, contentPath, archivePath, useCompression);

                mProgressDlg->show();
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

void MainWindow::onProgressCancelled() {
    mProgressCancelled = true;
}

void MainWindow::onProgressFinished() {
    if (mProgressDlg) {
        disconnect(mProgressDlg, &QProgressDialog::canceled, this, &MainWindow::onProgressCancelled);
        mProgressDlg->close();
    }
    mProgressCancelled = true;
}
