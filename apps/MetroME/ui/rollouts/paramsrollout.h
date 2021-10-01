#ifndef PARAMSROLLOUT_H
#define PARAMSROLLOUT_H

#include <QWidget>

namespace Ui {
class ParamsRollout;
}

class MetroSkeleton;

class ParamsRollout : public QWidget {
    Q_OBJECT

public:
    explicit ParamsRollout(QWidget* parent = nullptr);
    ~ParamsRollout();

    void    FillForTheSkeleton(MetroSkeleton* skeleton);

private slots:
    void    on_listParams_currentRowChanged(int currentRow);
    void    on_txtParamName_textEdited(const QString& text);
    void    on_spinParamBegin_valueChanged(double value);
    void    on_spinParamEnd_valueChanged(double value);
    void    on_spinParamLoop_valueChanged(double value);
    void    on_btnAdd_clicked();
    void    on_btnRemove_clicked();
    void    on_btnExport_clicked();
    void    on_btnImport_clicked();

private:
    void    UpdateItem(const int idx);
    void    RefreshAll(const int idxSelected);

private:
    Ui::ParamsRollout*  ui;
    MetroSkeleton*      mSkeleton;
};

#endif // PARAMSROLLOUT_H
