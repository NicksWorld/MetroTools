#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "objectpropertybrowser.h"

#include "common/mycommon.h"

class MetroTexturesDatabase;
class ImagePanel;
class MetroTextureInfoData;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void FillTexturesList();
    void AddOrReplaceTexture(bool replaceCurrent);

private slots:
    void on_lstTextures_currentRowChanged(int currentRow);
    void on_actionOpen_textures_bin_triggered();
    void on_actionSave_textures_bin_triggered();
    void on_actionAdd_texture_triggered();
    void on_actionRemove_texture_triggered();
    void on_actionShow_transparency_triggered();
    void on_actionCalculate_texture_average_colour_triggered();
    void onPropertyBrowserObjectPropertyChanged();

private:
    Ui::MainWindow*                     ui;
    ObjectPropertyBrowser*              mPropertyBrowser;
    ImagePanel*                         mImagePanel;

    StrongPtr<MetroTexturesDatabase>    mTexturesDB;
    fs::path                            mTexturesFolder;

    StrongPtr<MetroTextureInfoData>     mTextureInfoProps;
};

#endif // MAINWINDOW_H
