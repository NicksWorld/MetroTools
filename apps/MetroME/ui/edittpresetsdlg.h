#ifndef EDITTPRESETSDLG_H
#define EDITTPRESETSDLG_H

#include <QDialog>
#include <QTreeWidget>
#include <QListWidget>

#include "metro/MetroModel.h"


namespace Ui {
class EditTPresetsDlg;
}

class EditTPresetsDlg : public QDialog {
    Q_OBJECT

public:
    explicit EditTPresetsDlg(QWidget *parent = nullptr);
    ~EditTPresetsDlg();

    void    SetModel(RefPtr<MetroModelHierarchy> model);

private slots:
    void    on_lstPresets_currentRowChanged(int currentRow);
    void    on_lstPresets_itemChanged(QListWidgetItem* item);
    void    on_lstPresets_customContextMenuRequested(const QPoint& pos);
    void    on_txtHitPreset_textEdited(const QString& newText);
    void    on_txtVoice_textEdited(const QString& newText);
    void    on_treeItems_itemChanged(QTreeWidgetItem* item, int column);
    void    on_treeItems_customContextMenuRequested(const QPoint& pos);

private:
    Ui::EditTPresetsDlg*        ui;
    RefPtr<MetroModelHierarchy> mModel;
};

#endif // EDITTPRESETSDLG_H
