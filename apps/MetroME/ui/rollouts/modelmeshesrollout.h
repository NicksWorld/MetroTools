#ifndef MODELMESHESROLLOUT_H
#define MODELMESHESROLLOUT_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "mycommon.h"

namespace Ui {
class ModelMeshesRollout;
}

class ObjectPropertyBrowser;
class MaterialStringsProp;
class MetroModelBase;

class ModelMeshesRollout : public QWidget {
    Q_OBJECT

public:
    explicit ModelMeshesRollout(QWidget* parent = nullptr);
    ~ModelMeshesRollout();

    void    FillForTheModel(MetroModelBase* model);
    int     GetSelectedMeshIdx() const;

signals:
    void    SignalMeshSelectionChanged(int idx);
    void    SignalMeshPropertiesChanged();

private slots:
    void    on_treeModelHierarchy_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void    OnPropertyBrowserObjectPropertyChanged();

private:
    Ui::ModelMeshesRollout*         ui;
    MetroModelBase*                 mModel;
    int                             mSelectedGD;
    StrongPtr<MaterialStringsProp>  mMatStringsProp;
};

#endif // MODELMESHESROLLOUT_H
