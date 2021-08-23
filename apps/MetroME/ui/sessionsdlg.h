#ifndef SESSIONSDLG_H
#define SESSIONSDLG_H

#include <QDialog>
#include "mycommon.h"

namespace Ui {
class SessionsDlg;
}

class MetroSessionsList;

class SessionsDlg : public QDialog {
    Q_OBJECT

public:
    explicit SessionsDlg(QWidget *parent = nullptr);
    ~SessionsDlg();

    void            SetSessionsList(MetroSessionsList* list);

    bool            IsUseExistingSession() const;
    size_t          GetExistingSessionIdx() const;
    size_t          GetNewSessionGameVersion() const;
    const fs::path& GetNewSessionContentFolder() const;

public slots:
    void            OnWindowLoaded();

private slots:
    void            on_radioExistingSession_clicked();
    void            on_radiooNewSession_clicked();
    void            on_comboExistingSession_currentIndexChanged(int index);
    void            on_comboGameVersion_currentIndexChanged(int index);
    void            on_btnContentFolder_clicked();
    void            on_buttonBox_accepted();
    void            on_buttonBox_rejected();

private:
    Ui::SessionsDlg*    ui;
    MetroSessionsList*  mSessionsList;

    bool                mUseExistingSession;
    size_t              mExistingSessionIdx;
    size_t              mNewSessionGameVersion;
    fs::path            mNewSessionContentFolder;
};

#endif // SESSIONSDLG_H
