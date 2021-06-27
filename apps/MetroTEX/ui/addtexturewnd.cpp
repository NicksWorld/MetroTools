#include "addtexturewnd.h"
#include "ui_addtexturewnd.h"

#include <QFileDialog>


AddTextureWnd::AddTextureWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTextureWnd),
    mIsUpdatingUI(false)
{
    ui->setupUi(this);

    ui->grpAddExistingTexture->setEnabled(ui->chkUseExistingTexture->isChecked());
    ui->grpAddAndConvertTexture->setEnabled(ui->chkAddAndConvertTexture->isChecked());
}

AddTextureWnd::~AddTextureWnd() {
    delete ui;
}



bool AddTextureWnd::IsUseExistingTexture() const {
    return ui->chkUseExistingTexture->isChecked();
}

MyArray<fs::path> AddTextureWnd::GetAddedTextures() const {
    return mAddedTextures;
}

fs::path AddTextureWnd::GetOutputPath() const {
    return ui->txtAddAndConvertDestPath->text().toStdWString();
}

void AddTextureWnd::UpdateUI(QWidget* sender) {
    if (!mIsUpdatingUI) {
        mIsUpdatingUI = true;

        if (sender == ui->chkUseExistingTexture) {
            ui->chkAddAndConvertTexture->setChecked(!ui->chkUseExistingTexture->isChecked());
        } else {
            ui->chkUseExistingTexture->setChecked(!ui->chkAddAndConvertTexture->isChecked());
        }

        ui->grpAddExistingTexture->setEnabled(ui->chkUseExistingTexture->isChecked());
        ui->grpAddAndConvertTexture->setEnabled(ui->chkAddAndConvertTexture->isChecked());

        mIsUpdatingUI = false;
    }
}

void AddTextureWnd::on_chkUseExistingTexture_stateChanged(int) {
    this->UpdateUI(ui->chkUseExistingTexture);
}

void AddTextureWnd::on_chkAddAndConvertTexture_stateChanged(int) {
    this->UpdateUI(ui->chkAddAndConvertTexture);
}

void AddTextureWnd::on_btnBrowseExistingTexture_clicked() {
    QStringList names = QFileDialog::getOpenFileNames(this, tr("Open Metro 2033 texture..."), QString(), tr("Metro texture file (*.512;*.1024;*.2048;*.dds)"));
    if (!names.isEmpty()) {
        QString fullString;
        for (const QString& s : names) {
            fullString = fullString + s + ';';
        }

        // remove trailing `;`
        fullString.remove(fullString.length() - 1, 1);

        ui->txtExistingTexturePath->setText(fullString);
    }
}

void AddTextureWnd::on_btnBrowseSourceTexture_clicked() {
    QStringList names = QFileDialog::getOpenFileNames(this, tr("Open source image..."), QString(), tr("Image file (*.tga;*.png;*.bmp)"));
    if (!names.isEmpty()) {
        QString fullString;
        for (const QString& s : names) {
            fullString = fullString + s + ';';
        }

        // remove trailing `;`
        fullString.remove(fullString.length() - 1, 1);

        ui->txtAddAndConvertSourcePath->setText(fullString);
    }
}

void AddTextureWnd::on_btnBrowseOutputFolder_clicked() {
    QString folderName = QFileDialog::getExistingDirectory(this, tr("Choose output folder..."));
    if (folderName.length() > 3) {
        ui->txtAddAndConvertDestPath->setText(folderName);
    }
}

void AddTextureWnd::on_buttonBox_rejected() {
    this->setResult(QDialog::Rejected);
    this->close();
}

void AddTextureWnd::on_buttonBox_accepted() {
    mAddedTextures.clear();

    WideString text = this->IsUseExistingTexture() ? ui->txtExistingTexturePath->text().toStdWString() : ui->txtAddAndConvertSourcePath->text().toStdWString();
    MyArray<WStringView> names = WStrSplitViews(text, L';');

    for (const WStringView& s : names) {
        if (!s.empty()) {
            mAddedTextures.push_back(s);
        }
    }

    this->setResult(QDialog::Accepted);
    this->close();
}
