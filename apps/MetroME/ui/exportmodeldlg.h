#ifndef EXPORTMODELDLG_H
#define EXPORTMODELDLG_H

#include <QDialog>

namespace Ui {
class ExportModelDlg;
}

class MetroModelBase;

class ExportModelDlg : public QDialog {
    Q_OBJECT

public:
    explicit ExportModelDlg(QWidget* parent = nullptr);
    ~ExportModelDlg();

    void    SetModel(const MetroModelBase* model, const bool physicsAvailable);

    bool    IsExportAsStatic() const;
    bool    IsExportMeshesInlined() const;
    bool    IsExportSkeletonInlined() const;
    bool    IsSavePhysics() const;
    bool    IsOverrideModelVersion() const;
    int     GetOverrideModelVersion() const;

private slots:
    void    OnWindowLoaded();
    void    on_radioModelTypeStatic_toggled(bool checked);
    void    on_radioModelTypeAnimated_toggled(bool checked);
    void    on_chkInlineChildMeshes_stateChanged(int state);
    void    on_chkInlineSkeleton_stateChanged(int state);
    void    on_chkOverrideVersion_stateChanged(int state);
    void    on_comboOverrideVersion_currentIndexChanged(int index);
    void    on_buttonBox_accepted();
    void    on_buttonBox_rejected();

private:
    Ui::ExportModelDlg*     ui;
    const MetroModelBase*   mModel;
    bool                    mExportAsStatic;
    bool                    mExportMeshesInlined;
    bool                    mExportSkeletonInlined;
    bool                    mShouldOverrideModelVersion;
    int                     mOverrideModelVersion;
};

#endif // EXPORTMODELDLG_H
