#include "settingsdlg.h"
#include "ui_settingsdlg.h"

#include "mex_settings.h"

SettingsDlg::SettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDlg)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    const MEXSettings& s = MEXSettings::Get();

    // textures
    switch (s.extraction.textureFormat) {
        case MEXSettings::Extraction::TexFormat::Dds: {
            ui->radioTexAsDDS->setChecked(true);
        } break;
        case MEXSettings::Extraction::TexFormat::LegacyDds: {
            ui->radioTexAsLegacyDDS->setChecked(true);
        } break;
        case MEXSettings::Extraction::TexFormat::Tga: {
            ui->radioTexAsTGA->setChecked(true);
        } break;
        case MEXSettings::Extraction::TexFormat::Png: {
            ui->radioTexAsPNG->setChecked(true);
        } break;
    }

    // models
    switch (s.extraction.modelFormat) {
        case MEXSettings::Extraction::MdlFormat::Obj: {
            ui->radioMdlAsOBJ->setChecked(true);
        } break;
        case MEXSettings::Extraction::MdlFormat::Fbx: {
            ui->radioMdlAsFBX->setChecked(true);
        } break;
    }

    ui->chkMdlExportAnims->setChecked(s.extraction.modelSaveWithAnims);
    ui->chkMdlExportAnimsSeparate->setChecked(s.extraction.modelAnimsSeparate);
    ui->chkMdlSaveWithTextures->setChecked(s.extraction.modelSaveWithTextures);
    ui->chkMdlExcludeCollision->setChecked(s.extraction.modelExcludeCollision);
    ui->chkMdlSaveSurfaceTextures->setChecked(s.extraction.modelSaveSurfaceSet);
    ui->chkMdlExportLods->setChecked(s.extraction.modelSaveLods);

    // sounds
    switch (s.extraction.soundFormat) {
        case MEXSettings::Extraction::SndFormat::Ogg: {
            ui->radioSndAsOGG->setChecked(true);
        } break;
        case MEXSettings::Extraction::SndFormat::Wav: {
            ui->radioSndAsWAV->setChecked(true);
        } break;
    }

    // stuff
    ui->chkExportAskEveryTime->setChecked(s.extraction.askEveryTime);
}

SettingsDlg::~SettingsDlg() {
    delete ui;
}

void SettingsDlg::on_btnBoxMain_accepted() {
    MEXSettings& s = MEXSettings::Get();

    // textures
    if (ui->radioTexAsDDS->isChecked()) {
        s.extraction.textureFormat = MEXSettings::Extraction::TexFormat::Dds;
    } else if (ui->radioTexAsLegacyDDS->isChecked()) {
        s.extraction.textureFormat = MEXSettings::Extraction::TexFormat::LegacyDds;
    } else if (ui->radioTexAsTGA->isChecked()) {
        s.extraction.textureFormat = MEXSettings::Extraction::TexFormat::Tga;
    } else {
        s.extraction.textureFormat = MEXSettings::Extraction::TexFormat::Png;
    }

    // models
    if (ui->radioMdlAsOBJ->isChecked()) {
        s.extraction.modelFormat = MEXSettings::Extraction::MdlFormat::Obj;
    } else {
        s.extraction.modelFormat = MEXSettings::Extraction::MdlFormat::Fbx;
    }

    s.extraction.modelSaveWithAnims = ui->chkMdlExportAnims->isChecked();
    s.extraction.modelAnimsSeparate = ui->chkMdlExportAnimsSeparate->isChecked();
    s.extraction.modelSaveWithTextures = ui->chkMdlSaveWithTextures->isChecked();
    s.extraction.modelExcludeCollision = ui->chkMdlExcludeCollision->isChecked();
    s.extraction.modelSaveSurfaceSet = ui->chkMdlSaveSurfaceTextures->isChecked();
    s.extraction.modelSaveLods = ui->chkMdlExportLods->isChecked();

    // sounds
    if (ui->radioSndAsOGG->isChecked()) {
        s.extraction.soundFormat = MEXSettings::Extraction::SndFormat::Ogg;
    } else {
        s.extraction.soundFormat = MEXSettings::Extraction::SndFormat::Wav;
    }

    // stuff
    s.extraction.askEveryTime = ui->chkExportAskEveryTime->isChecked();

    s.Save();

    this->close();
}

void SettingsDlg::on_btnBoxMain_rejected() {
    this->close();
}
