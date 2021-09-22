#include "sessionsdlg.h"
#include "ui_sessionsdlg.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#include "mycommon.h"
#include "../MetroSessions.h"

SessionsDlg::SessionsDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SessionsDlg)
    , mSessionsList(nullptr)
    , mUseExistingSession(false)
    , mExistingSessionIdx(0)
    , mNewSessionGameVersion(scast<size_t>(MetroGameVersion::Unknown))
    , mNewSessionContentFolder{}
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, SLOT(OnWindowLoaded()));
}

SessionsDlg::~SessionsDlg() {
    delete ui;
}



void SessionsDlg::SetSessionsList(MetroSessionsList* list) {
    mSessionsList = list;

    const size_t numSession = list->GetNumSessions();
    if (numSession) {
        for (size_t i = 0; i < numSession; ++i) {
            const MetroSession& session = list->GetSession(i);
            ui->comboExistingSession->addItem(QString("%1 - %2").arg(QString::fromStdString(MetroGameVersionNames[scast<size_t>(session.GetGameVersion())]))
                                                                .arg(QString::fromStdWString(session.GetContentFolder().wstring())));
        }
        ui->comboExistingSession->setCurrentIndex(0);

        ui->radioExistingSession->setChecked(true);
        this->on_radioExistingSession_clicked();
    } else {
        ui->radiooNewSession->setChecked(true);
        this->on_radiooNewSession_clicked();
    }

    for (size_t i = 0; i < scast<size_t>(MetroGameVersion::NumVersions); ++i) {
        ui->comboGameVersion->addItem(QString::fromStdString(MetroGameVersionNames[i]));
    }
}

bool SessionsDlg::IsUseExistingSession() const {
    return mUseExistingSession;
}

size_t SessionsDlg::GetExistingSessionIdx() const {
    return mExistingSessionIdx;
}

size_t SessionsDlg::GetNewSessionGameVersion() const {
    return mNewSessionGameVersion;
}

const fs::path& SessionsDlg::GetNewSessionContentFolder() const {
    return mNewSessionContentFolder;
}


void SessionsDlg::OnWindowLoaded() {
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());
}


void SessionsDlg::on_radioExistingSession_clicked() {
    mUseExistingSession = true;
    ui->comboExistingSession->setEnabled(true);
    ui->groupNewSession->setEnabled(false);
    mExistingSessionIdx = scast<size_t>(ui->comboExistingSession->currentIndex());
}

void SessionsDlg::on_radiooNewSession_clicked() {
    mUseExistingSession = false;
    ui->comboExistingSession->setEnabled(false);
    ui->groupNewSession->setEnabled(true);
}

void SessionsDlg::on_comboExistingSession_currentIndexChanged(int index) {
    mExistingSessionIdx = scast<size_t>(index);
}

void SessionsDlg::on_comboGameVersion_currentIndexChanged(int index) {
    mNewSessionGameVersion = scast<size_t>(index);
}

void SessionsDlg::on_btnContentFolder_clicked() {
    QString folder = QFileDialog::getExistingDirectory(this, tr("Select game unpacked content folder"));
    if (!folder.isEmpty()) {
        fs::path folderPath = fs::absolute(folder.toStdWString());
        if (folderPath.filename() != L"content") {
            QMessageBox::critical(this, this->windowTitle(), tr("The selected folder must be the root \"content\" folder!"));
        } else {
            ui->txtNewSessionContentFolder->setText(QString::fromStdWString(folderPath.wstring()));
        }
    }
}

void SessionsDlg::on_buttonBox_accepted() {
    if (!mUseExistingSession) {
        mNewSessionContentFolder = ui->txtNewSessionContentFolder->text().toStdWString();
    }
}

void SessionsDlg::on_buttonBox_rejected() {

}
