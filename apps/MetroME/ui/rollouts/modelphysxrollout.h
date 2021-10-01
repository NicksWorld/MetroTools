#ifndef MODELPHYSXROLLOUT_H
#define MODELPHYSXROLLOUT_H

#include <QWidget>

namespace Ui {
class ModelPhysXRollout;
}

class MetroModelBase;

class ModelPhysXRollout : public QWidget {
    Q_OBJECT

public:
    explicit ModelPhysXRollout(QWidget* parent = nullptr);
    ~ModelPhysXRollout();

    void    FillForTheModel(MetroModelBase* model);

private slots:
    void    on_textPhysXLinks_textChanged();

private:
    Ui::ModelPhysXRollout*  ui;
    MetroModelBase*         mModel;
};

#endif // MODELPHYSXROLLOUT_H
