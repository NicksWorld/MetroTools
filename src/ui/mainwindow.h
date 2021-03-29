#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>

#include <shlobj_core.h>    // IProgressDialog
#include <thread>           // std::thread

#include "metro/MetroTypes.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImagePanel;
class RenderPanel;
class ImageInfoPanel;
class ModelInfoPanel;

enum class FileType : size_t {
    Unknown,
    Folder,
    FolderBin,
    Bin,
    BinArchive,
    BinEditable,
    Texture,
    Model,
    Motion,
    Level,
    Sound,
    Localization,
    LightProbe
};

enum class PanelType : size_t {
    Texture,
    Model,
    Sound,
    Localization
};

struct FileExtractionCtx {
    MyHandle    file;
    FileType    type;

    size_t      customOffset;
    size_t      customLength;
    CharString  customFileName;

    // models
    bool        mdlSaveAsObj;
    bool        mdlSaveAsFbx;
    bool        mdlSaveWithAnims;
    bool        mdlAnimsSeparate;
    bool        mdlSaveWithTextures;
    bool        mdlExcludeCollision;
    bool        mdlSaveLods;
    // textures
    bool        txUseBC3;
    bool        txSaveAsDds;
    bool        txSaveAsTga;
    bool        txSaveAsPng;
    // sounds
    bool        sndSaveAsOgg;
    bool        sndSaveAsWav;

    // batch
    bool        batch;
    bool        raw;
    size_t      numFilesTotal;
    size_t      progress;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_archive_triggered();
    void on_actionOpen_triggered();
    void on_actionShow_transparency_triggered();
    void on_actionSettings_triggered();
    void on_treeFiles_itemCollapsed(QTreeWidgetItem* item);
    void on_treeFiles_itemExpanded(QTreeWidgetItem* item);
    void on_treeFiles_customContextMenuRequested(const QPoint& pos);
    void on_treeFiles_itemSelectionChanged();
    void on_treeFilterTimer_tick();
    void on_txtFilterTree_textEdited(const QString& newText);

private:
    void UpdateFilesList();
    void AddFoldersRecursive(MyHandle folder, QTreeWidgetItem* rootItem, const MyHandle configBinFile);
    void AddBinaryArchive(MyHandle file, QTreeWidgetItem* rootItem);
    void UpdateNodeIcon(QTreeWidgetItem* node);
    void FilterTree(QTreeWidgetItem* node, const QString& text);
    void DetectFileAndShow(MyHandle file);
    void ShowTexture(MyHandle file);
    void ShowModel(MyHandle file);
    void ShowLevel(MyHandle file);
    void ShowLocalization(MyHandle file);
    void SwitchViewPanel(const PanelType t);
    void SwitchInfoPanel(const PanelType t);

    // context menus
    void ShowContextMenuFolder(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuTexture(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuModel(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuSound(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuLocalization(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuBin(QTreeWidgetItem* node, const QPoint& pos);
    void ShowContextMenuConfigBin(QTreeWidgetItem* node, const QPoint& pos, const bool enableModified);
    void ShowContextMenuRAW(QTreeWidgetItem* node, const QPoint& pos);
    // extraction
    void OnExtractFolderClicked(bool withConversion);

    // extraction helpers
    void EnsureExtractionOptions();
    CharString DecideTextureExtension(const FileExtractionCtx& ctx);
    CharString MakeFileOutputName(MyHandle file, const FileExtractionCtx& ctx);
    void TextureSaveHelper(const fs::path& folderPath, const FileExtractionCtx& ctx, const CharString& name);
    bool ExtractFile(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractTexture(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractSurfaceSet(const FileExtractionCtx& ctx, const MetroSurfaceDescription& surface, const fs::path& outFolder);
    bool ExtractModel(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractMotion(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractSound(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractLocalization(const FileExtractionCtx& ctx, const fs::path& outPath);
    bool ExtractFolderComplete(const FileExtractionCtx& ctx, const fs::path& outPath);
    void ExtractionProcessFunc(const fs::path& folderPath);

    // property panels
    // model props
private slots:
    void OnModelInfoMotionsSelectedIndexChanged(int selection);
    void OnModelInfoLodsSelectedIndexChanged(int selection);
    void OnModelInfoPlayStopAnimClicked();
    void OnModelInfoInfoClicked();
    void OnModelInfoExportMotionClicked();

private:
    Ui::MainWindow*             ui;
    QIcon                       mIconFolderClosed;
    QIcon                       mIconFolderOpened;
    QIcon                       mIconFile;
    QIcon                       mIconImage;
    QIcon                       mIconLevel;
    QIcon                       mIconLocalization;
    QIcon                       mIconSound;
    QIcon                       mIconModel;
    QIcon                       mIconBin;
    QIcon                       mIconBinArchive;

    // Filterable tree
    QList<QTreeWidgetItem*>     mOriginalTreeRootNodes;
    bool                        mIsTreeFiltering;
    StrongPtr<QTimer>           mTreeFilteringTimer;

    // Viewers panels
    ImagePanel*                 mImagePanel;
    RenderPanel*                mRenderPanel;
    QTableWidget*               mLocalizationPanel;

    // Info panels
    ImageInfoPanel*             mImageInfoPanel;
    ModelInfoPanel*             mModelInfoPanel;

    // extraction
    FileExtractionCtx           mExtractionCtx;
    IProgressDialog*            mExtractionProgressDlg;
    std::thread                 mExtractionThread;
};
#endif // MAINWINDOW_H
