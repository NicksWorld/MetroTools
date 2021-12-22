#include "edittpresetsdlg.h"
#include "ui_edittpresetsdlg.h"

#include <QMenu>
#include <QMessageBox>

EditTPresetsDlg::EditTPresetsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTPresetsDlg)
{
    ui->setupUi(this);

    ui->treeItems->setHeaderLabels({"mtl_name", "texture", "shader"});
    ui->splitter->setSizes({ 100, 300 });
}

EditTPresetsDlg::~EditTPresetsDlg() {
    delete ui;
}


void EditTPresetsDlg::SetModel(RefPtr<MetroModelHierarchy> model) {
    mModel = model;

    ui->lstPresets->clear();
    ui->treeItems->clear();
    ui->txtHitPreset->clear();
    ui->txtVoice->clear();

    const size_t numPresets = mModel->GetNumTPresets();
    if (numPresets) {
        for (size_t i = 0; i < numPresets; ++i) {
            const MetroModelTPreset& tpreset = mModel->GetTPreset(i);
            ui->lstPresets->addItem(QString::fromStdString(tpreset.name));
        }

        ui->lstPresets->setCurrentRow(0);
    }
}

void EditTPresetsDlg::on_lstPresets_currentRowChanged(int currentRow) {
    const size_t numPresets = mModel->GetNumTPresets();
    if (currentRow >= 0 && currentRow < numPresets) {
        const MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(currentRow));

        ui->txtHitPreset->setText(QString::fromStdString(tpreset.hit_preset));
        ui->txtVoice->setText(QString::fromStdString(tpreset.voice));

        ui->treeItems->clear();
        int idx = 0;
        for (auto& item : tpreset.items) {
            QTreeWidgetItem* treeItem = new QTreeWidgetItem({
                QString::fromStdString(item.mtl_name),
                QString::fromStdString(item.t_dst),
                QString::fromStdString(item.s_dst)
            });
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsEditable);
            ui->treeItems->addTopLevelItem(treeItem);
        }
    }
}

void EditTPresetsDlg::on_lstPresets_itemChanged(QListWidgetItem* item) {
    const int idx = ui->lstPresets->row(item);
    const size_t numPresets = mModel->GetNumTPresets();
    if (idx >= 0 && idx < numPresets) {
        MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(idx));

        if (!item->text().isEmpty()) {
            tpreset.name = item->text().toStdString();
        } else {
            item->setText(QString::fromStdString(tpreset.name));
        }
    }
}

void EditTPresetsDlg::on_lstPresets_customContextMenuRequested(const QPoint& pos) {
    QListWidgetItem* clickedItem = ui->lstPresets->itemAt(pos);

    QMenu menu;
    QAction* addItem = menu.addAction(tr("Add new preset"));
    QAction* deleteItem = (clickedItem != nullptr) ? menu.addAction(tr("Delete this preset")) : nullptr;

    QAction* selectedAction = menu.exec(ui->lstPresets->mapToGlobal(pos));
    if (selectedAction && selectedAction == deleteItem) {
        QApplication::beep();
        const int answer = QMessageBox::question(this, this->windowTitle(), tr("Are tou sure you want to delete \"%1\" ?").arg(clickedItem->text()));
        if (QMessageBox::Yes == answer) {
            const int idx = ui->lstPresets->row(clickedItem);
            mModel->DeleteTPreset(scast<size_t>(idx));
            clickedItem = ui->lstPresets->takeItem(idx);
            MySafeDelete(clickedItem);
            if (ui->lstPresets->count() > 0) {
                ui->lstPresets->setCurrentRow(0);
            }
        }
    } else if (selectedAction == addItem) {
        QListWidgetItem* newItem = new QListWidgetItem();
        QString newPresetName = QString("new_preset_%1").arg(ui->lstPresets->count());

        MetroModelTPreset tpreset = {};
        tpreset.name = newPresetName.toStdString();
        mModel->AddTPreset(tpreset);

        newItem->setText(newPresetName);
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        ui->lstPresets->addItem(newItem);
        ui->lstPresets->setCurrentItem(newItem);
    }
}

void EditTPresetsDlg::on_txtHitPreset_textEdited(const QString& newText) {
    const int idx = ui->lstPresets->currentRow();
    const size_t numPresets = mModel->GetNumTPresets();
    if (idx >= 0 && idx < numPresets) {
        MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(idx));
        tpreset.hit_preset = newText.toStdString();
    }
}

void EditTPresetsDlg::on_txtVoice_textEdited(const QString& newText) {
    const int idx = ui->lstPresets->currentRow();
    const size_t numPresets = mModel->GetNumTPresets();
    if (idx >= 0 && idx < numPresets) {
        MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(idx));
        tpreset.voice = newText.toStdString();
    }
}

void EditTPresetsDlg::on_treeItems_itemChanged(QTreeWidgetItem* item, int column) {
    const int idx = ui->lstPresets->currentRow();
    const size_t numPresets = mModel->GetNumTPresets();
    if (idx >= 0 && idx < numPresets) {
        MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(idx));
        const int itemIdx = ui->treeItems->indexOfTopLevelItem(item);

        if (itemIdx >= 0 && itemIdx < tpreset.items.size()) {
            QString newText = item->text(column);
            if (0 == column) {
                tpreset.items[itemIdx].mtl_name = newText.toStdString();
            } else if (1 == column) {
                tpreset.items[itemIdx].t_dst = newText.toStdString();
            } else {
                tpreset.items[itemIdx].s_dst = newText.toStdString();
            }
        }
    }
}

void EditTPresetsDlg::on_treeItems_customContextMenuRequested(const QPoint& pos) {
    const int idx = ui->lstPresets->currentRow();
    const size_t numPresets = mModel->GetNumTPresets();
    if (idx >= 0 && idx < numPresets) {
        MetroModelTPreset& tpreset = mModel->GetTPreset(scast<size_t>(idx));

        QTreeWidgetItem* item = ui->treeItems->itemAt(pos);

        QMenu menu;
        QAction* addItem = menu.addAction(tr("Add new item"));
        QAction* deleteItem = (item != nullptr) ? menu.addAction(tr("Delete this item")) : nullptr;

        QAction* selectedAction = menu.exec(ui->treeItems->mapToGlobal(pos));
        if (selectedAction && selectedAction == deleteItem) {
            const int itemIdx = ui->treeItems->indexOfTopLevelItem(item);
            if (itemIdx >= 0 && itemIdx < tpreset.items.size()) {
                QApplication::beep();
                const int answer = QMessageBox::question(this, this->windowTitle(), tr("Are tou sure you want to delete \"%1\" ?").arg(QString::fromStdString(tpreset.items[itemIdx].mtl_name)));
                if (QMessageBox::Yes == answer) {
                    tpreset.items.erase(tpreset.items.begin() + itemIdx);
                    item = ui->treeItems->takeTopLevelItem(itemIdx);
                    MySafeDelete(item);
                }
            }
        } else if (selectedAction == addItem) {
            tpreset.items.push_back({ "mtl name", "", ""});
            QTreeWidgetItem* treeItem = new QTreeWidgetItem({ "mtl name", "", "" });
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsEditable);
            ui->treeItems->addTopLevelItem(treeItem);
        }
    }
}
