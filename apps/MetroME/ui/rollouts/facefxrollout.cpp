#include "facefxrollout.h"
#include "ui_facefxrollout.h"

#include "metro/MetroSkeleton.h"

FaceFXRollout::FaceFXRollout(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FaceFXRollout)
    , mSkeleton(nullptr)
{
    ui->setupUi(this);
}

FaceFXRollout::~FaceFXRollout() {
    delete ui;
}


void FaceFXRollout::FillForTheSkeleton(MetroSkeleton* skeleton) {
    mSkeleton = skeleton;

    if (mSkeleton) {
        ui->txtFaceFX->setText(QString::fromStdString(skeleton->GetFaceFX()));
    } else {
        ui->txtFaceFX->setText("");
    }
}

void FaceFXRollout::on_txtFaceFX_textEdited(const QString& text) {
    if (mSkeleton) {
        mSkeleton->SetFaceFX(text.toStdString());
    }
}
