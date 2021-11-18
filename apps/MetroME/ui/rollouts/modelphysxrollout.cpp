#include "modelphysxrollout.h"
#include "ui_modelphysxrollout.h"

#include "metro/MetroModel.h"

ModelPhysXRollout::ModelPhysXRollout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModelPhysXRollout)
{
    ui->setupUi(this);
}

ModelPhysXRollout::~ModelPhysXRollout() {
    delete ui;
}


void ModelPhysXRollout::FillForTheModel(MetroModelBase* model) {
    mModel = model;

    if (mModel && mModel->IsSkeleton()) {
        auto links = scast<MetroModelSkeleton*>(mModel)->GetPhysXLinks();
        CharString linksString;
        for (const CharString& s : links) {
            if (!s.empty()) {
                linksString = linksString + s + ',';
            }
        }
        if (!linksString.empty()) {
            linksString.pop_back(); // remove trailing ','
        }

        ui->textPhysXLinks->setPlainText(QString::fromStdString(linksString));
    }
}

void ModelPhysXRollout::on_textPhysXLinks_textChanged() {
    if (mModel && mModel->IsSkeleton()) {
        StringArray links = StrSplit(ui->textPhysXLinks->toPlainText().toStdString(), ',');
        scast<MetroModelSkeleton*>(mModel)->SetPhysXLinks(links);
    }
}
