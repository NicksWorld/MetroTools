#include "modelinfopanel.h"
#include "ui_modelinfopanel.h"

ModelInfoPanel::ModelInfoPanel(QWidget* parent)
    : QFrame(parent)
    , ui(new Ui::MdlInfoPanel)
{
    ui->setupUi(this);
}
ModelInfoPanel::~ModelInfoPanel()
{
}

void ModelInfoPanel::SetModelTypeText(const QString& text) {
    ui->lblMdlPropType->setText(text);
}

void ModelInfoPanel::SetNumVerticesText(const QString& text) {
    ui->lblMdlPropVertices->setText(text);
}

void ModelInfoPanel::SetNumTrianglesText(const QString& text) {
    ui->lblMdlPropTriangles->setText(text);
}

void ModelInfoPanel::SetNumJointsText(const QString& text) {
    ui->lblMdlPropJoints->setText(text);
}

void ModelInfoPanel::SetNumAnimationsText(const QString& text) {
    ui->lblMdlPropNumAnims->setText(text);
}

void ModelInfoPanel::SetPlayStopButtonText(const QString& text) {
    ui->btnMdlPropPlayStopAnim->setText(text);
}

void ModelInfoPanel::ClearMotionsList() {
    ui->lstMdlPropMotions->clear();
}

void ModelInfoPanel::AddMotionToList(const QString& name) {
    ui->lstMdlPropMotions->addItem(name);
}

void ModelInfoPanel::ClearLodsList() {
    ui->lstLods->clear();
}

void ModelInfoPanel::AddLodIdxToList(const int lodIdx) {
    ui->lstLods->addItem(QString("Lod %1").arg(lodIdx));
}

void ModelInfoPanel::SelectLod(const int lodIdx) {
    ui->lstLods->setCurrentIndex(lodIdx);
}


void ModelInfoPanel::on_btnMdlPropPlayStopAnim_clicked(bool checked) {
    Q_UNUSED(checked);
    emit playStopClicked();
}

void ModelInfoPanel::on_btnModelInfo_clicked(bool checked) {
    Q_UNUSED(checked);
    emit modelInfoClicked();
}

void ModelInfoPanel::on_btnModelExportMotion_clicked(bool checked) {
    Q_UNUSED(checked);
    emit exportMotionClicked();
}

void ModelInfoPanel::on_lstMdlPropMotions_itemSelectionChanged() {
    const int idx = ui->lstMdlPropMotions->currentRow();
    if (idx >= 0 && idx < ui->lstMdlPropMotions->count()) {
        emit motionsListSelectionChanged(idx);
    }
}

void ModelInfoPanel::on_lstLods_currentIndexChanged(int index) {
    if (index >= 0 && index < ui->lstLods->count()) {
        emit lodsListSelectionChanged(index);
    }
}
