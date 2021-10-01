#include "paramsrollout.h"
#include "ui_paramsrollout.h"

#include "metro/MetroSkeleton.h"
#include "metro/reflection/MetroReflection.h"

#include <QFileDialog>



ParamsRollout::ParamsRollout(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ParamsRollout)
    , mSkeleton(nullptr)
{
    ui->setupUi(this);
}

ParamsRollout::~ParamsRollout() {
    delete ui;
}


void ParamsRollout::FillForTheSkeleton(MetroSkeleton* skeleton) {
    ui->listParams->clear();

    mSkeleton = skeleton;

    if (mSkeleton) {
        const size_t numParams = mSkeleton->GetNumParams();
        if (numParams) {
            for (size_t i = 0; i < numParams; ++i) {
                const MetroSkelParam& param = skeleton->GetSkelParam(i);
                QString fullString = QString("\"%1\", b=%2, e=%3, l=%4").arg(QString::fromStdString(param.name)).arg(param.b).arg(param.e).arg(param.loop);
                ui->listParams->addItem(fullString);
            }

            ui->listParams->setCurrentRow(0);
        }
    }
}

void ParamsRollout::on_listParams_currentRowChanged(int currentRow) {
    if (mSkeleton && currentRow >= 0) {
        const MetroSkelParam& param = mSkeleton->GetSkelParam(scast<size_t>(currentRow));
        ui->txtParamName->setText(QString::fromStdString(param.name));
        ui->spinParamBegin->setValue(param.b);
        ui->spinParamEnd->setValue(param.e);
        ui->spinParamLoop->setValue(param.loop);
    } else {
        ui->txtParamName->setText("");
        ui->spinParamBegin->setValue(0.0);
        ui->spinParamEnd->setValue(0.0);
        ui->spinParamLoop->setValue(0.0);
    }
}

void ParamsRollout::on_txtParamName_textEdited(const QString& text) {
    const int idx = ui->listParams->currentRow();
    if (mSkeleton && idx >= 0) {
        MetroSkelParam param = mSkeleton->GetSkelParam(scast<size_t>(idx));
        param.name = text.toStdString();
        mSkeleton->SetSkelParam(scast<size_t>(idx), param);
        this->UpdateItem(idx);
    }
}

void ParamsRollout::on_spinParamBegin_valueChanged(double value) {
    const int idx = ui->listParams->currentRow();
    if (mSkeleton && idx >= 0) {
        MetroSkelParam param = mSkeleton->GetSkelParam(scast<size_t>(idx));
        param.b = scast<float>(value);
        mSkeleton->SetSkelParam(scast<size_t>(idx), param);
        this->UpdateItem(idx);
    }
}

void ParamsRollout::on_spinParamEnd_valueChanged(double value) {
    const int idx = ui->listParams->currentRow();
    if (mSkeleton && idx >= 0) {
        MetroSkelParam param = mSkeleton->GetSkelParam(scast<size_t>(idx));
        param.e = scast<float>(value);
        mSkeleton->SetSkelParam(scast<size_t>(idx), param);
        this->UpdateItem(idx);
    }
}

void ParamsRollout::on_spinParamLoop_valueChanged(double value) {
    const int idx = ui->listParams->currentRow();
    if (mSkeleton && idx >= 0) {
        MetroSkelParam param = mSkeleton->GetSkelParam(scast<size_t>(idx));
        param.loop = scast<float>(value);
        mSkeleton->SetSkelParam(scast<size_t>(idx), param);
        this->UpdateItem(idx);
    }
}

void ParamsRollout::on_btnAdd_clicked() {
    if (mSkeleton) {
        mSkeleton->AddSkelParam({});
        this->RefreshAll(ui->listParams->count() - 1);
    }
}

void ParamsRollout::on_btnRemove_clicked() {
    const int idx = ui->listParams->currentRow();
    if (mSkeleton && idx >= 0) {
        mSkeleton->RemoveSkelParam(scast<size_t>(idx));
        this->RefreshAll(idx > 0 ? (idx - 1) : 0);
    }
}

void ParamsRollout::UpdateItem(const int idx) {
    if (mSkeleton && idx >= 0 && idx < ui->listParams->count()) {
        QListWidgetItem* item = ui->listParams->item(idx);
        const MetroSkelParam& param = mSkeleton->GetSkelParam(scast<size_t>(idx));
        QString fullString = QString("\"%1\", b=%2, e=%3, l=%4").arg(QString::fromStdString(param.name)).arg(param.b).arg(param.e).arg(param.loop);
        item->setText(fullString);
    }
}

void ParamsRollout::RefreshAll(const int idxSelected) {
    this->FillForTheSkeleton(mSkeleton);
    ui->listParams->setCurrentRow(idxSelected);
}

void ParamsRollout::on_btnExport_clicked() {
    if (mSkeleton) {
        QString saveName = QFileDialog::getSaveFileName(nullptr, tr("Where to save SkelParams"), QString(), tr("Json files (*.json);;All files (*.*)"));
        if (!saveName.isEmpty()) {
            MyArray<MetroSkelParam> skelParams;
            for (size_t i = 0; i < mSkeleton->GetNumParams(); ++i) {
                skelParams.push_back(mSkeleton->GetSkelParam(i));
            }

            MetroReflectionJsonWriteStream jsonWriter;
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(jsonWriter, skelParams);
            CharString jsonString = jsonWriter.WriteToString();
            OSWriteFile(saveName.toStdWString(), jsonString.data(), jsonString.length());
        }
    }
}

void ParamsRollout::on_btnImport_clicked() {
    if (mSkeleton) {
        QString loadName = QFileDialog::getOpenFileName(nullptr, tr("Open SkelParams file"), QString(), tr("Json files (*.json);;All files (*.*)"));
        if (!loadName.isEmpty()) {
            MemStream stream = OSReadFile(loadName.toStdWString());
            CharString jsonString;
            jsonString.assign(rcast<const char*>(stream.Data()), stream.Length());

            MyArray<MetroSkelParam> skelParams;
            MetroReflectionJsonReadStream jsonReader(jsonString);
            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(jsonReader, skelParams);

            mSkeleton->ReplaceSkelParams(skelParams);
            this->RefreshAll(0);
        }
    }
}
