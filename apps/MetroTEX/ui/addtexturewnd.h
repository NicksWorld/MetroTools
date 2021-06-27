#ifndef ADDTEXTUREWND_H
#define ADDTEXTUREWND_H

#include <QDialog>

#include "common/mycommon.h"

namespace Ui {
class AddTextureWnd;
}

class AddTextureWnd : public QDialog
{
    Q_OBJECT

public:
    explicit AddTextureWnd(QWidget *parent = nullptr);
    ~AddTextureWnd();

    bool                IsUseExistingTexture() const;
    MyArray<fs::path>   GetAddedTextures() const;
    fs::path            GetOutputPath() const;

private:
    void                UpdateUI(QWidget* sender);

private slots:
    void on_chkUseExistingTexture_stateChanged(int arg1);
    void on_chkAddAndConvertTexture_stateChanged(int arg1);
    void on_btnBrowseExistingTexture_clicked();
    void on_btnBrowseSourceTexture_clicked();
    void on_btnBrowseOutputFolder_clicked();
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

private:
    Ui::AddTextureWnd*  ui;
    MyArray<fs::path>   mAddedTextures;
    bool                mIsUpdatingUI;
};

#endif // ADDTEXTUREWND_H
