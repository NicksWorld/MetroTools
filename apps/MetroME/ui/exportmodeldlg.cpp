#include "exportmodeldlg.h"
#include "ui_exportmodeldlg.h"

#include <QMessageBox>
#include <QTimer>

#include "metro/MetroTypes.h"
#include "metro/MetroModel.h"


ExportModelDlg::ExportModelDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ExportModelDlg)
    , mModel(nullptr)
    , mExportAsStatic(false)
    , mExportMeshesInlined(false)
    , mExportSkeletonInlined(false)
    , mShouldOverrideModelVersion(false)
    , mOverrideModelVersion(0)
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, SLOT(OnWindowLoaded()));
}

ExportModelDlg::~ExportModelDlg() {
    delete ui;
}


void ExportModelDlg::SetModel(const MetroModelBase* model) {
    mModel = model;
    if (mModel) {
        if (mModel->IsSkeleton()) {
            ui->radioModelTypeAnimated->setChecked(true);
            ui->chkInlineSkeleton->setEnabled(true);
        } else {
            ui->radioModelTypeStatic->setChecked(true);
        }

        const MetroGameVersion gameVersion = MetroModelBase::GetGameVersionFromModelVersion(mModel->GetModelVersion());
        ui->comboOverrideVersion->setEnabled(true);
        ui->comboOverrideVersion->setCurrentIndex(scast<int>(gameVersion));
        ui->comboOverrideVersion->setEnabled(false);
    }
}

bool ExportModelDlg::IsExportAsStatic() const {
    return mExportAsStatic;
}

bool ExportModelDlg::IsExportMeshesInlined() const {
    return mExportMeshesInlined;
}

bool ExportModelDlg::IsExportSkeletonInlined() const {
    return mExportSkeletonInlined;
}

bool ExportModelDlg::IsOverrideModelVersion() const {
    return mShouldOverrideModelVersion;
}

int ExportModelDlg::GetOverrideModelVersion() const {
    return mOverrideModelVersion;
}


void ExportModelDlg::OnWindowLoaded() {
    // make this dialog fixed-size
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());

    ui->comboOverrideVersion->clear();
    for (size_t i = 0; i < scast<size_t>(MetroGameVersion::NumVersions); ++i) {
        ui->comboOverrideVersion->addItem(QString::fromStdString(MetroGameVersionNames[i]));
    }
}


void ExportModelDlg::on_radioModelTypeStatic_toggled(bool checked) {
    if (checked) {
        ui->chkInlineSkeleton->setChecked(false);
        ui->chkInlineSkeleton->setEnabled(false);
    }
}

void ExportModelDlg::on_radioModelTypeAnimated_toggled(bool checked) {
    if (checked) {
        ui->chkInlineSkeleton->setEnabled(true);
    }
}

void ExportModelDlg::on_chkInlineChildMeshes_stateChanged(int state) {
    mExportMeshesInlined = (Qt::Checked == state);
}

void ExportModelDlg::on_chkInlineSkeleton_stateChanged(int state) {
    mExportSkeletonInlined = (Qt::Checked == state);
}

void ExportModelDlg::on_chkOverrideVersion_stateChanged(int state) {
    mShouldOverrideModelVersion = (Qt::Checked == state);
    ui->comboOverrideVersion->setEnabled(mShouldOverrideModelVersion);
}

void ExportModelDlg::on_comboOverrideVersion_currentIndexChanged(int index) {
    mOverrideModelVersion = index;
}

void ExportModelDlg::on_buttonBox_accepted() {
    QDialog::accept();
}

void ExportModelDlg::on_buttonBox_rejected() {
    QDialog::reject();
}
