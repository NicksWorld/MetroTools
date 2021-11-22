#include "exportfbxdlg.h"
#include "ui_exportfbxdlg.h"

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"

ExportFBXDlg::ExportFBXDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportFBXDlg)
{
    ui->setupUi(this);
    ui->label->setPixmap(QIcon(":/imgs/fbx.svg").pixmap(ui->label->size()));
}

ExportFBXDlg::~ExportFBXDlg() {
    delete ui;
}

void ExportFBXDlg::SetModel(MetroModelBase* model) {
    const MetroGameVersion gameVersion = MetroModelBase::GetGameVersionFromModelVersion(model->GetModelVersion());

    if (!model->IsSkeleton()) {
        ui->chkExportSkeleton->setEnabled(false);
        ui->chkExportSkeleton->setChecked(false);
    } else {
        ui->chkExportSkeleton->setEnabled(true);
        ui->chkExportSkeleton->setChecked(true);
    }

    ui->chkExportShadowGeometry->setEnabled(gameVersion >= MetroGameVersion::Arktika1);
    ui->chkExportShadowGeometry->setChecked(gameVersion >= MetroGameVersion::Arktika1);

    if (model->GetLodCount() > 0) {
        ui->chkExportLODs->setEnabled(true);
        ui->chkExportLODs->setChecked(true);
    } else {
        ui->chkExportLODs->setEnabled(false);
        ui->chkExportLODs->setChecked(false);
    }
}

bool ExportFBXDlg::GetExportLODs() const {
    return ui->chkExportLODs->isChecked();
}

bool ExportFBXDlg::GetExportShadowGeometry() const {
    return ui->chkExportShadowGeometry->isChecked();
}

bool ExportFBXDlg::GetExportSkeleton() const {
    return ui->chkExportSkeleton->isChecked();
}


void ExportFBXDlg::on_chkExportLODs_stateChanged(int state) {
}


void ExportFBXDlg::on_chkExportShadowGeometry_stateChanged(int state) {
}


void ExportFBXDlg::on_chkExportSkeleton_stateChanged(int state) {
}


void ExportFBXDlg::on_buttonBox_accepted() {
    this->accept();
}

