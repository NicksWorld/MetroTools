#include "maintoolbar.h"
#include "ui_maintoolbar.h"
#include "mex_settings.h"

#include <QMenu>

MainToolbar::MainToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainToolbar)
{
    ui->setupUi(this);

    mOpenArchiveEmptyAction.setIcon(ui->tbtnOpenArchive->icon());
    mOpenArchiveEmptyAction.setText("");
    mOpenArchiveEmptyAction.setToolTip(ui->tbtnOpenArchive->toolTip());

    mOpenGameFolderEmptyAction.setIcon(ui->tbtnOpenGameFolder->icon());
    mOpenGameFolderEmptyAction.setText("");
    mOpenGameFolderEmptyAction.setToolTip(ui->tbtnOpenGameFolder->toolTip());

    ui->tbtnOpenArchive->setDefaultAction(&mOpenArchiveEmptyAction);
    ui->tbtnOpenGameFolder->setDefaultAction(&mOpenGameFolderEmptyAction);

    MEXSettings& settings = MEXSettings::Get();

    SetOpenArchiveHistory(settings.openHistory.archives);
    SetOpenGameFolderHistory(settings.openHistory.folders);
}

MainToolbar::~MainToolbar() {
    delete ui;
}

void MainToolbar::SetOpenArchiveHistory(const WStringArray& history) {
    if (!history.empty()) {
        ui->tbtnOpenArchive->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu();
        for (long i = history.size() - 1; i >= 0; --i) {
            menu->addAction(QString::fromStdWString(history[i]));
        }
        ui->tbtnOpenArchive->setMenu(menu);
        ui->tbtnOpenArchive->setIcon(mOpenArchiveEmptyAction.icon());
        ui->tbtnOpenArchive->setToolTip(mOpenArchiveEmptyAction.toolTip());
    } else {
        ui->tbtnOpenArchive->setMenu(nullptr);
        ui->tbtnOpenArchive->setPopupMode(QToolButton::DelayedPopup);
    }
}

void MainToolbar::SetOpenGameFolderHistory(const WStringArray& history) {
    if (!history.empty()) {
        ui->tbtnOpenGameFolder->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu();
        for (long i = history.size() - 1; i >= 0; --i) {
            menu->addAction(QString::fromStdWString(history[i]));
        }
        ui->tbtnOpenGameFolder->setMenu(menu);
        ui->tbtnOpenGameFolder->setDefaultAction(menu->actions().first());
        ui->tbtnOpenGameFolder->setIcon(mOpenGameFolderEmptyAction.icon());
        ui->tbtnOpenGameFolder->setToolTip(mOpenGameFolderEmptyAction.toolTip());
    } else {
        ui->tbtnOpenGameFolder->setMenu(nullptr);
        ui->tbtnOpenGameFolder->setPopupMode(QToolButton::DelayedPopup);
    }
}

void MainToolbar::on_tbtnOpenArchive_triggered(QAction* action) {
    QString path = (action != nullptr) ? action->text() : "";
    emit OnOpenArchiveTriggered(path);
}

void MainToolbar::on_tbtnOpenGameFolder_triggered(QAction* action) {
    QString path = (action != nullptr) ? action->text() : "";
    emit OnOpenGameFolderTriggered(path);
}

void MainToolbar::on_tbtnShowTransparency_toggled(bool checked) {
    emit OnShowTransparencyTriggered(checked);
}

void MainToolbar::on_tbtnSettings_clicked() {
    emit OnSettingsTriggered();
}

void MainToolbar::on_tbtnAbout_clicked() {
    emit OnAboutTriggered();
}