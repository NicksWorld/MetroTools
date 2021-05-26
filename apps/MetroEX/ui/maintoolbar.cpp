#include "maintoolbar.h"
#include "ui_maintoolbar.h"

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
}

MainToolbar::~MainToolbar() {
    delete ui;
}

void MainToolbar::SetOpenArchiveHistory(const WStringArray& history) {
    if (!history.empty()) {
        ui->tbtnOpenArchive->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu();
        for (auto& str : history) {
            menu->addAction(QString::fromStdWString(str));
        }
        ui->tbtnOpenArchive->setMenu(menu);
        ui->tbtnOpenArchive->setDefaultAction(menu->actions().first());
        ui->tbtnOpenArchive->setIcon(mOpenArchiveEmptyAction.icon());
        ui->tbtnOpenArchive->setToolTip(mOpenArchiveEmptyAction.toolTip());
    } else {
        ui->tbtnOpenArchive->setMenu(nullptr);
        ui->tbtnOpenArchive->setPopupMode(QToolButton::DelayedPopup);
        ui->tbtnOpenArchive->setDefaultAction(&mOpenArchiveEmptyAction);
    }
}

void MainToolbar::SetOpenGameFolderHistory(const WStringArray& history) {
    if (!history.empty()) {
        ui->tbtnOpenGameFolder->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu* menu = new QMenu();
        for (auto& str : history) {
            menu->addAction(QString::fromStdWString(str));
        }
        ui->tbtnOpenGameFolder->setMenu(menu);
        ui->tbtnOpenGameFolder->setDefaultAction(menu->actions().first());
        ui->tbtnOpenGameFolder->setIcon(mOpenGameFolderEmptyAction.icon());
        ui->tbtnOpenGameFolder->setToolTip(mOpenGameFolderEmptyAction.toolTip());
    } else {
        ui->tbtnOpenGameFolder->setMenu(nullptr);
        ui->tbtnOpenGameFolder->setPopupMode(QToolButton::DelayedPopup);
        ui->tbtnOpenGameFolder->setDefaultAction(&mOpenGameFolderEmptyAction);
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
