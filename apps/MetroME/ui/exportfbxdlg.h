#ifndef EXPORTFBXDLG_H
#define EXPORTFBXDLG_H

#include <QDialog>
#include "mycommon.h"

class MetroModelBase;

namespace Ui {
class ExportFBXDlg;
}

class ExportFBXDlg : public QDialog {
    Q_OBJECT

public:
    explicit ExportFBXDlg(QWidget *parent = nullptr);
    ~ExportFBXDlg();

    void    SetModel(MetroModelBase* model);

    bool    GetExportLODs() const;
    bool    GetExportShadowGeometry() const;
    bool    GetExportSkeleton() const;
    bool    GetExportAllAttachPoints() const;

private slots:
    void    on_chkExportLODs_stateChanged(int state);
    void    on_chkExportShadowGeometry_stateChanged(int state);
    void    on_chkExportSkeleton_stateChanged(int state);
    void    on_chkExportAllAttachPoints_stateChanged(int state);
    void    on_buttonBox_accepted();

private:
    Ui::ExportFBXDlg*   ui;
};

#endif // EXPORTFBXDLG_H
