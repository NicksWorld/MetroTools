#include "motionsrollout.h"
#include "ui_motionsrollout.h"

#include "metro/MetroSkeleton.h"

MotionsRollout::MotionsRollout(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MotionsRollout)
    , mSkeleton(nullptr)
{
    ui->setupUi(this);
}

MotionsRollout::~MotionsRollout() {
    delete ui;
}


void MotionsRollout::FillForTheSkeleton(MetroSkeleton* skeleton) {
    mSkeleton = skeleton;

    if (mSkeleton) {
        ui->textMotions->setPlainText(QString::fromStdString(mSkeleton->GetMotionsStr()));
    } else {
        ui->textMotions->setPlainText("");
    }
}

void MotionsRollout::on_textMotions_textChanged() {
    if (mSkeleton) {
        CharString str = ui->textMotions->toPlainText().toStdString();
        mSkeleton->SetMotionsStr(str);
    }
}
