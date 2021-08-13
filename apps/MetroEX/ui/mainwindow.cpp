#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>

#include "imagepanel.h"
#include "renderpanel.h"
#include "imageinfopanel.h"
#include "modelinfopanel.h"
#include "nodes/scriptscene.h"
#include "maintoolbar.h"

#include "settingsdlg.h"
#include "mex_settings.h"

#include "metro/MetroContext.h"
#include "metro/MetroTexture.h"
#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroSound.h"
#include "metro/MetroLocalization.h"
#include "metro/MetroLevel.h"
#include "metro/MetroBinArchive.h"
#undef IN
#undef OUT
#include "metro/reflection/MetroReflection.h"
#include "metro/scripts/MetroScript.h"

#include "exporters/ExporterOBJ.h"
#include "exporters/ExporterFBX.h"

#include "engine/Renderer.h"
#include "engine/ResourcesManager.h"

#include <nodes/FlowScene>
#include <nodes/FlowView>

class MyTreeWidgetItem : public QTreeWidgetItem {
public:
    explicit MyTreeWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit MyTreeWidgetItem(const QStringList &strings, int type = Type) : QTreeWidgetItem(strings, type) {}
    explicit MyTreeWidgetItem(QTreeWidget *treeview, int type = Type) : QTreeWidgetItem(treeview, type) {}
    MyTreeWidgetItem(QTreeWidget *treeview, const QStringList &strings, int type = Type) : QTreeWidgetItem(treeview, strings, type) {}
    MyTreeWidgetItem(QTreeWidget *treeview, QTreeWidgetItem *after, int type = Type) : QTreeWidgetItem(treeview, after, type) {}
    explicit MyTreeWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    MyTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    MyTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, int type = Type) : QTreeWidgetItem(parent, after, type) {}
    MyTreeWidgetItem(const QTreeWidgetItem &other) : QTreeWidgetItem(other) {}
    virtual ~MyTreeWidgetItem() {}

    QList<MyTreeWidgetItem*> FindInChildren(const QString key) {
        QList<MyTreeWidgetItem*> result;

        const int numChildren = this->childCount();
        for (int i = 0; i < numChildren; ++i) {
            MyTreeWidgetItem* child = scast<MyTreeWidgetItem*>(this->child(i));
            if (child && (child->text(0) == key || child->toolTip(0) == key)) {
                result.push_back(child);
            }
        }

        return result;
    }

    bool operator<(const QTreeWidgetItem& other) const override {
        const int l = this->childCount() ? 1 : 0;
        const int r = other.childCount() ? 1 : 0;
        if (l == r) {
            return this->text(0) < other.text(0);
        } else {
            return l > r;
        }
    }
};

static uint64_t MakeNodeTag(const MyHandle fileHandle, const FileType type, const size_t subIdx = kInvalidValue) {
    return ((scast<uint64_t>(type) & 0xFF) << 56) | (scast<uint64_t>(subIdx & 0xFFFFFF) << 32) | (fileHandle & 0xFFFFFFFF);
}

static MyHandle GetFileHandleFromTag(const uint64_t tag) {
    const MyHandle result = scast<MyHandle>(tag & 0xFFFFFFFF);
    return (result == kInvalidValue32) ? kInvalidHandle : result;
}

static FileType GetFileTypeFromTag(const uint64_t tag) {
    return scast<FileType>((tag >> 56) & 0xFF);
}

static size_t GetFileSubIdxFromTag(const uint64_t tag) {
    const size_t result = scast<size_t>((tag >> 32) & 0xFFFFFF);
    return (result == 0xFFFFFF) ? kInvalidValue : result;
}


static FileType DetectFileType(const MetroFSPath& file) {
    FileType result = FileType::Unknown;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    const CharString& name = mfs.GetName(file);

    if (StrEndsWith(name, ".dds") ||
        StrEndsWith(name, ".512") || StrEndsWith(name, ".512c") ||
        StrEndsWith(name, ".1024") || StrEndsWith(name, ".1024c") ||
        StrEndsWith(name, ".2048") || StrEndsWith(name, ".2048c") ||
        StrEndsWith(name, ".4096")) {
        result = FileType::Texture;
    } else if (name == "level.bin") {
        result = FileType::Level;
    } else if (StrEndsWith(name, ".bin")) {
        result = FileType::Bin;
    } else if (StrEndsWith(name, ".model") || StrEndsWith(name, ".mesh")) {
        result = FileType::Model;
    } else if (StrEndsWith(name, ".lprobe")) {
        result = FileType::LightProbe;
    } else if (StrEndsWith(name, ".m2") || StrEndsWith(name, ".motion")) {
        result = FileType::Motion;
    } else if (StrEndsWith(name, ".vba") || StrEndsWith(name, ".ogg")) {
        result = FileType::Sound;
    } else if (StrEndsWith(name, ".lng")) {
        result = FileType::Localization;
    }

    return result;
}




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mToolbar(new MainToolbar)
    , mIsTreeFiltering(false)
    , mImagePanel(nullptr)
    , mRenderPanel(nullptr)
    , mLocalizationPanel(nullptr)
    , mImageInfoPanel(nullptr)
    , mModelInfoPanel(nullptr)
    , mExtractionCtx{}
    , mExtractionProgressDlg(nullptr)
{
    ui->setupUi(this);

    ui->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->toolBar->addWidget(mToolbar);
    connect(mToolbar, &MainToolbar::OnOpenArchiveTriggered, this, &MainWindow::on_OpenArchive_triggered);
    connect(mToolbar, &MainToolbar::OnOpenGameFolderTriggered, this, &MainWindow::on_OpenGameFolder_triggered);
    connect(mToolbar, &MainToolbar::OnShowTransparencyTriggered, this, &MainWindow::on_ShowTransparency_triggered);
    connect(mToolbar, &MainToolbar::OnSettingsTriggered, this, &MainWindow::on_Settings_triggered);
    connect(mToolbar, &MainToolbar::OnAboutTriggered, this, &MainWindow::on_About_triggered);

    ui->treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);

    // Viewers panels
    mImagePanel = new ImagePanel(ui->panelViewers);
    mImagePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mImagePanel->setGeometry(QRect(0, 0, ui->panelViewers->width(), ui->panelViewers->height()));
    ui->panelViewers->layout()->addWidget(mImagePanel);
    mImagePanel->hide();

    mRenderPanel = new RenderPanel(ui->panelViewers);
    mRenderPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRenderPanel->setGeometry(QRect(0, 0, ui->panelViewers->width(), ui->panelViewers->height()));
    ui->panelViewers->layout()->addWidget(mRenderPanel);
    mRenderPanel->hide();

    bool deviceIsOk = u4a::Renderer::Get().CreateDevice();
    if (!deviceIsOk) {
        QMessageBox::critical(this, this->windowTitle(), tr("Failed to create DirectX 11 graphics!\n3D viewer will be unavailable."));
    } else {
        if (!u4a::Renderer::Get().Initialize()) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to finish DirectX 11 renderer initialization!\n3D viewer will be unavailable."));
        } else {
            u4a::ResourcesManager::Get().Initialize();
        }

        if (!mRenderPanel->Initialize()) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to initialize Render panel!\n3D viewer will be unavailable."));
        }
    }

    mLocalizationPanel = new QTableWidget(ui->panelViewers);
    mLocalizationPanel->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLocalizationPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLocalizationPanel->setGeometry(QRect(0, 0, ui->panelViewers->width(), ui->panelViewers->height()));
    ui->panelViewers->layout()->addWidget(mLocalizationPanel);
    mLocalizationPanel->hide();

    SetNodeStyle();
    mCurScriptScene = std::make_unique<QtNodes::FlowScene>();
    mVisualScriptPanel = new QtNodes::FlowView(mCurScriptScene.get(), ui->panelViewers);
    mVisualScriptPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mVisualScriptPanel->setGeometry(QRect(0, 0, ui->panelViewers->width(), ui->panelViewers->height()));
    ui->panelViewers->layout()->addWidget(mVisualScriptPanel);
    mVisualScriptPanel->hide();

    // Info panels
    mImageInfoPanel = new ImageInfoPanel(ui->panelMetaProps);
    mImageInfoPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mImageInfoPanel->setGeometry(QRect(0, 0, ui->panelMetaProps->width(), ui->panelMetaProps->height()));
    ui->panelMetaProps->layout()->addWidget(mImageInfoPanel);
    mImageInfoPanel->hide();

    mModelInfoPanel = new ModelInfoPanel(ui->panelMetaProps);
    mModelInfoPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mModelInfoPanel->setGeometry(QRect(0, 0, ui->panelMetaProps->width(), ui->panelMetaProps->height()));
    ui->panelMetaProps->layout()->addWidget(mModelInfoPanel);
    mModelInfoPanel->hide();
    connect(mModelInfoPanel, &ModelInfoPanel::motionsListSelectionChanged, this, &MainWindow::OnModelInfoMotionsSelectedIndexChanged);
    connect(mModelInfoPanel, &ModelInfoPanel::playStopClicked, this, &MainWindow::OnModelInfoPlayStopAnimClicked);
    connect(mModelInfoPanel, &ModelInfoPanel::modelInfoClicked, this, &MainWindow::OnModelInfoInfoClicked);
    connect(mModelInfoPanel, &ModelInfoPanel::exportMotionClicked, this, &MainWindow::OnModelInfoExportMotionClicked);
    connect(mModelInfoPanel, &ModelInfoPanel::lodsListSelectionChanged, this, &MainWindow::OnModelInfoLodsSelectedIndexChanged);

    mIconFolderClosed = QIcon("://imgs/FolderClosed_32x.png");
    mIconFolderOpened = QIcon("://imgs/FolderOpened_32x.png");
    mIconFile = QIcon("://imgs/Document_32x.png");
    mIconImage = QIcon("://imgs/Image_32x.png");
    mIconLevel = QIcon("://imgs/Level_32x.png");
    mIconLocalization = QIcon("://imgs/Localization_32x.png");
    mIconSound = QIcon("://imgs/Sound_32x.png");
    mIconModel = QIcon("://imgs/Model_32x.png");
    mIconBin = QIcon("://imgs/SettingsFile_32x.png");
    mIconBinArchive = QIcon("://imgs/LibrarySettings_32x.png");

    mTreeFilteringTimer = MakeStrongPtr<QTimer>();
    mTreeFilteringTimer->setInterval(1000); // 1 second
    connect(mTreeFilteringTimer.get(), SIGNAL(timeout()), this, SLOT(on_treeFilterTimer_tick()));
    mTreeFilteringTimer->start();
}

MainWindow::~MainWindow() {
    if (mExtractionThread.joinable()) {
        mExtractionThread.join();
    }

    delete ui;
}


void MainWindow::on_OpenArchive_triggered(QString path) {
    QString name = path.isEmpty() ? QFileDialog::getOpenFileName(this, tr("Open Metro archive file..."), QString::fromStdWString(GetLastArchivePathFromHistory()), tr("Metro archive files (*.vfx *.vfx0 *.vfi)")) : path;
    if (!name.isEmpty()) {
        AddArchiveToHistory(name.toStdWString());
        if (MetroContext::Get().InitFromSingleArchive(name.toStdWString())) {
            this->UpdateFilesList();
        }
    }
}

void MainWindow::on_OpenGameFolder_triggered(QString path) {
    // AddFolderToHistory(name.toStdWString());
}

void MainWindow::on_ShowTransparency_triggered(bool checked) {
    mImagePanel->ShowTransparency(checked);
}

void MainWindow::on_Settings_triggered() {
    SettingsDlg dlg(this);
    dlg.setWindowIcon(this->windowIcon());
    dlg.exec();
}

void MainWindow::on_About_triggered() {
    QMessageBox::aboutQt(this, this->windowTitle());
}



void MainWindow::on_treeFiles_itemCollapsed(QTreeWidgetItem* item) {
    const uint64_t tag = item->data(0, Qt::UserRole).value<uint64_t>();
    const FileType fileType = GetFileTypeFromTag(tag);

    if (fileType == FileType::Folder || fileType == FileType::FolderBin) {
        item->setIcon(0, mIconFolderClosed);
    }

    ui->treeFiles->resizeColumnToContents(0);
}

void MainWindow::on_treeFiles_itemExpanded(QTreeWidgetItem* item) {
    const uint64_t tag = item->data(0, Qt::UserRole).value<uint64_t>();
    const FileType fileType = GetFileTypeFromTag(tag);

    if (fileType == FileType::Folder || fileType == FileType::FolderBin) {
        item->setIcon(0, mIconFolderOpened);
    }

    ui->treeFiles->resizeColumnToContents(0);

}

void MainWindow::on_treeFilterTimer_tick() {
    mTreeFilteringTimer->stop();

    ui->treeFiles->clear();

    const QString& text = ui->txtFilterTree->text().trimmed();
    if (text.isEmpty()) {
        mIsTreeFiltering = false;
        ui->treeFiles->addTopLevelItems(mOriginalTreeRootNodes);
    } else {
        mIsTreeFiltering = true;
        for (QTreeWidgetItem* node : mOriginalTreeRootNodes) {
            QTreeWidgetItem* clonedNode = node->clone();
            this->FilterTree(clonedNode, text);
            ui->treeFiles->addTopLevelItem(clonedNode);
            ui->treeFiles->expandItem(clonedNode);
        }
    }
}

void MainWindow::on_txtFilterTree_textEdited(const QString& /*newText*/) {
    if (mOriginalTreeRootNodes.empty()) {
        const int numRootNodes = ui->treeFiles->topLevelItemCount();
        mOriginalTreeRootNodes.reserve(numRootNodes);
        for (int i = 0; i < numRootNodes; ++i) {
            mOriginalTreeRootNodes.push_back(ui->treeFiles->topLevelItem(i)->clone());
        }
    }

    if (!mOriginalTreeRootNodes.empty()) {
        mTreeFilteringTimer->stop();
        mTreeFilteringTimer->start();
    }
}

void MainWindow::on_treeFiles_customContextMenuRequested(const QPoint& pos) {
    QList<QTreeWidgetItem*> selection = ui->treeFiles->selectedItems();
    if (!selection.empty()) {
        QTreeWidgetItem* node = selection.front();
        const uint64_t tag = node->data(0, Qt::UserRole).value<uint64_t>();
        const FileType fileType = GetFileTypeFromTag(tag);
        MyHandle file = GetFileHandleFromTag(tag);
        const size_t subFileIdx = GetFileSubIdxFromTag(tag);
        const bool isSubFile = subFileIdx != kInvalidValue;

        mExtractionCtx = {};
        mExtractionCtx.file = MetroFSPath(file);
        mExtractionCtx.type = fileType;
        mExtractionCtx.customOffset = kInvalidValue;
        mExtractionCtx.customLength = kInvalidValue;
        mExtractionCtx.customFileName = kEmptyString;

        const MetroConfigsDatabase& mcfgdb = MetroContext::Get().GetConfigsDB();

        QPoint globalPos = ui->treeFiles->mapToGlobal(pos);

        switch (fileType) {
            case FileType::Folder: {
                this->ShowContextMenuFolder(node, globalPos);
            } break;

            case FileType::Texture: {
                this->ShowContextMenuTexture(node, globalPos);
            } break;

            case FileType::Model: {
                this->ShowContextMenuModel(node, globalPos);
            } break;

            case FileType::Sound: {
                this->ShowContextMenuSound(node, globalPos);
            } break;

            case FileType::Localization: {
                this->ShowContextMenuLocalization(node, globalPos);
            } break;

            case FileType::Bin: {
                if (isSubFile) {
                    const MetroConfigsDatabase::ConfigInfo& ci = mcfgdb.GetFileByIdx(subFileIdx);

                    mExtractionCtx.customOffset = ci.offset;
                    mExtractionCtx.customLength = ci.length;
                    mExtractionCtx.customFileName = node->text(0).toStdString();
                    this->ShowContextMenuBin(node, globalPos);
                } else {
                    this->ShowContextMenuConfigBin(node, globalPos, mcfgdb.IsDirty());
                }
            } break;

            case FileType::FolderBin: {
            } break;

            default:
                this->ShowContextMenuRAW(node, globalPos);
                break;
        }
    }
}

void MainWindow::on_treeFiles_itemSelectionChanged() {
    QList<QTreeWidgetItem*> selection = ui->treeFiles->selectedItems();
    if (!selection.empty()) {
        QTreeWidgetItem* node = selection.front();
        const uint64_t tag = node->data(0, Qt::UserRole).value<uint64_t>();
        MyHandle file = GetFileHandleFromTag(tag);
        FileType type = GetFileTypeFromTag(tag);
        size_t subIdx = GetFileSubIdxFromTag(tag);

        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
        if (!mfs.Empty()) {
            const bool isFolder = mfs.IsFolder(MetroFSPath(file)) || type == FileType::FolderBin || type == FileType::BinArchive;
            if (!isFolder) {
                this->DetectFileAndShow(file, subIdx);
            }
        }
    }
}



void MainWindow::UpdateFilesList() {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    if (!mfs.Empty()) {
        ui->txtFilterTree->clear();
        ui->treeFiles->clear();
        for (QTreeWidgetItem* node : mOriginalTreeRootNodes) {
            delete node;
        }
        mOriginalTreeRootNodes.clear();

        // Get idx of config.bin
        const MetroFSPath& configBinFile = mfs.FindFile("content\\config.bin");

        QString rootName = tr("FileSystem");
        if (mfs.IsSingleArchive()) {
            rootName = QString::fromStdString(mfs.GetArchiveName(0));
        }

        MyTreeWidgetItem* rootNode = new MyTreeWidgetItem(QStringList(rootName));
        rootNode->setToolTip(0, rootNode->text(0));
        rootNode->setIcon(0, mIconFolderClosed);
        ui->treeFiles->addTopLevelItem(rootNode);
        size_t rootIdx = 0;

        //mOriginalRootNode = rootNode;

        rootNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(rootIdx, FileType::Folder)));
        //UpdateNodeIcon(rootNode);

        const MetroFSPath& rootDir = mfs.GetRootFolder();
        for (MyHandle child = mfs.GetFirstChild(rootDir.fileHandle); child != kInvalidHandle; child = mfs.GetNextChild(child)) {
            if (mfs.IsFolder(MetroFSPath(child))) {
                this->AddFoldersRecursive(child, rootNode, configBinFile.fileHandle);
            } else {
                const FileType fileType = DetectFileType(MetroFSPath(child));

                MyTreeWidgetItem* fileNode = new MyTreeWidgetItem(QStringList(QString::fromStdString(mfs.GetName(MetroFSPath(child)))));
                fileNode->setToolTip(0, fileNode->text(0));
                fileNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(child, fileType)));
                this->UpdateNodeIcon(fileNode);
                rootNode->addChild(fileNode);
            }
        }

        ui->treeFiles->sortItems(0, Qt::AscendingOrder);
        ui->treeFiles->resizeColumnToContents(0);
    }
}

void MainWindow::AddFoldersRecursive(MyHandle folder, QTreeWidgetItem* rootItem, const MyHandle configBinFile) {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();;

    // Add root folder
    MyTreeWidgetItem* dirLeafNode = new MyTreeWidgetItem(QStringList(QString::fromStdString(mfs.GetName(MetroFSPath(folder)))));
    dirLeafNode->setToolTip(0, dirLeafNode->text(0));
    dirLeafNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(folder, FileType::Folder)));
    dirLeafNode->setIcon(0, mIconFolderClosed);
    rootItem->addChild(dirLeafNode);

    // Add files and folders inside
    for (auto child = mfs.GetFirstChild(folder); child != kInvalidHandle; child = mfs.GetNextChild(child)) {
        if (mfs.IsFolder(MetroFSPath(child))) {
            // Add folder to list
            this->AddFoldersRecursive(child, dirLeafNode, configBinFile);
        } else {
            // Add file to list
            if (child == configBinFile) {
                // config.bin
                this->AddBinaryArchive(child, dirLeafNode);
            } else {
                //====> any other file
                const FileType fileType = DetectFileType(MetroFSPath(child));

                MyTreeWidgetItem* fileNode = new MyTreeWidgetItem(QStringList(QString::fromStdString(mfs.GetName(MetroFSPath(child)))));
                fileNode->setToolTip(0, fileNode->text(0));
                fileNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(child, fileType)));
                this->UpdateNodeIcon(fileNode);
                dirLeafNode->addChild(fileNode);
            }
        }
    }
}

void MainWindow::AddBinaryArchive(MyHandle file, QTreeWidgetItem* rootItem) {
    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    MyTreeWidgetItem* fileNode = new MyTreeWidgetItem(QStringList(QString::fromStdString(mfs.GetName(MetroFSPath(file)))));
    fileNode->setToolTip(0, fileNode->text(0));
    fileNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(file, FileType::BinArchive)));
    this->UpdateNodeIcon(fileNode);
    rootItem->addChild(fileNode);

    const MetroConfigsDatabase& mcfgdb = MetroContext::Get().GetConfigsDB();

    for (size_t idx = 0, numFiles = mcfgdb.GetNumFiles(); idx < numFiles; ++idx) {
        const MetroConfigsDatabase::ConfigInfo& ci = mcfgdb.GetFileByIdx(idx);

        const bool isNameDecrypted = !ci.nameStr.empty();

        QString fileName = isNameDecrypted ? QString::fromStdString(ci.nameStr) : QString("unkn\\CRC32_0x%1.bin").arg(ci.nameCRC, 8, 16, QLatin1Char('0'));

        MyTreeWidgetItem* lastNode = fileNode; // folder to add file

        QStringList pathArray = fileName.split(QLatin1Char('\\'));
        fileName = pathArray.back();

        // Add all sub-folders
        QString curPath = pathArray.front();
        for (qsizetype i = 0; i < (pathArray.length() - 1); ++i) {
            QList<MyTreeWidgetItem*> folderNodes = lastNode->FindInChildren(curPath);
            if (folderNodes.empty()) {
                // Create new folder node
                QString folderName = pathArray[i];

                MyTreeWidgetItem* newNode = new MyTreeWidgetItem(QStringList(folderName));
                newNode->setToolTip(0, curPath);
                newNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(file, FileType::FolderBin, 0)));
                lastNode->addChild(newNode);
                lastNode = newNode;
                this->UpdateNodeIcon(lastNode);
            } else {
                // Use existing node folder
                lastNode = folderNodes[0];
            }

            curPath += QString("\\") + pathArray[i + 1];
        }

        // Add binary file
        MyTreeWidgetItem* chunkNode = new MyTreeWidgetItem(QStringList(fileName));
        chunkNode->setToolTip(0, fileName);
        chunkNode->setData(0, Qt::UserRole, QVariant::fromValue<uint64_t>(MakeNodeTag(file, FileType::Bin, idx)));
        this->UpdateNodeIcon(chunkNode);
        lastNode->addChild(chunkNode);
    }
}


void MainWindow::UpdateNodeIcon(QTreeWidgetItem* node) {
    const uint64_t tag = node->data(0, Qt::UserRole).value<uint64_t>();
    const FileType fileType = GetFileTypeFromTag(tag);

    switch (fileType) {
        case FileType::Folder:
        case FileType::FolderBin: {
            node->setIcon(0, mIconFolderClosed);
        } break;

        case FileType::Unknown: {
            node->setIcon(0, mIconFile);
        } break;

        case FileType::Bin: {
            node->setIcon(0, mIconBin);
        } break;

        case FileType::BinArchive: {
            node->setIcon(0, mIconBinArchive);
        } break;

        case FileType::BinEditable: {
            node->setIcon(0, mIconBin);
        } break;

        case FileType::Model: {
            node->setIcon(0, mIconModel);
        } break;

        case FileType::Texture: {
            node->setIcon(0, mIconImage);
        } break;

        case FileType::Sound: {
            node->setIcon(0, mIconSound);
        } break;

        case FileType::Localization: {
            node->setIcon(0, mIconLocalization);
        } break;

        case FileType::Level: {
            node->setIcon(0, mIconLevel);
        } break;

        default:
            node->setIcon(0, mIconFile);
            break;
    }
}

void MainWindow::FilterTree(QTreeWidgetItem* node, const QString& text) {
    QList<QTreeWidgetItem*> nodesToRemove;

    for (int i = 0; i < node->childCount(); ++i) {
        QTreeWidgetItem* child = node->child(i);
        if (child->childCount() > 0) {
            this->FilterTree(child, text);

            if (child->childCount() == 0) {
                nodesToRemove.push_back(child);
            } else {
                ui->treeFiles->expandItem(child);
            }
        } else if (!child->text(0).contains(text)) {
            nodesToRemove.push_back(child);
        }
    }

    for (QTreeWidgetItem* n : nodesToRemove) {
        node->removeChild(n);
    }
}

void MainWindow::DetectFileAndShow(MyHandle file, size_t subIdx) {
    const bool isFolder = MetroContext::Get().GetFilesystem().IsFolder(MetroFSPath(file));
    if (!isFolder) {
        const FileType fileType = DetectFileType(MetroFSPath(file));

        switch (fileType) {
            case FileType::Texture: {
                this->ShowTexture(file);
            } break;

            case FileType::Model: {
                this->ShowModel(file);
            } break;

            //case FileType::LightProbe: {
            //    this->ShowLightProbe(file);
            //} break;

            //case FileType::Sound: {
            //    this->ShowSound(file);
            //} break;

            case FileType::Level: {
                this->ShowLevel(file);
            } break;

            case FileType::Localization: {
                this->ShowLocalization(file);
            } break;

            case FileType::Bin: {
                this->ShowBin(file, subIdx);
            } break;

            default:
                break;
        }
    }
}

void MainWindow::ShowTexture(MyHandle file) {
    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(MetroFSPath(file));
    if (stream) {
        const CharString& fileName = MetroContext::Get().GetFilesystem().GetName(MetroFSPath(file));

        MetroTexture texture;
        if (texture.LoadFromData(stream, fileName)) {
            if (texture.IsCubemap()) {
                this->SwitchViewPanel(PanelType::Model);
                //mRenderPanel->SetCubemap(&texture);
            } else {
                this->SwitchViewPanel(PanelType::Texture);

                BytesArray pixels;
                texture.GetRGBA(pixels);
                mImagePanel->SetImage(pixels.data(), texture.GetWidth(), texture.GetHeight());
            }

            this->SwitchInfoPanel(PanelType::Texture);

            mImageInfoPanel->SetCompressionText(QString::fromStdString(MetroTexture::PixelFormatNames[scast<uint32_t>(texture.GetFormat())]));
            mImageInfoPanel->SetWidthText(QString("%1").arg(texture.GetWidth()));
            mImageInfoPanel->SetHeightText(QString("%1").arg(texture.GetHeight()));
            mImageInfoPanel->SetMipsText(QString("%1").arg(texture.GetNumMips()));
        }
    }
}

void MainWindow::ShowModel(MyHandle file) {
    this->SwitchViewPanel(PanelType::Model);
    this->SwitchInfoPanel(PanelType::Model);

    RefPtr<MetroModelBase> mdl = MetroModelFactory::CreateModelFromFile(MetroFSPath(file), MetroModelLoadParams::LoadEverything);
    if (mdl) {
        mRenderPanel->SetModel(nullptr);

        mModelInfoPanel->ClearLodsList();
        mModelInfoPanel->AddLodIdxToList(0);

        if (mdl->GetLodCount() > 0) {
            mModelInfoPanel->AddLodIdxToList(1);
            if (mdl->GetLodCount() > 1) {
                mModelInfoPanel->AddLodIdxToList(2);
            }
        }

        mModelInfoPanel->SelectLod(0);

        mRenderPanel->SetModel(mdl);

        mModelInfoPanel->ClearMotionsList();
        if (mdl->IsSkeleton()) {
            RefPtr<MetroModelSkeleton> skelMdl = SCastRefPtr<MetroModelSkeleton>(mdl);
            RefPtr<MetroSkeleton> skeleton = skelMdl->GetSkeleton();

            size_t numMotions = 0;

            if (skeleton) {
                numMotions = skeleton->GetNumMotions();
                for (size_t i = 0; i < numMotions; ++i) {
                    const CharString& motionName = skeleton->GetMotionName(i);
                    mModelInfoPanel->AddMotionToList(QString::fromStdString(motionName));
                }
            }

            mModelInfoPanel->SetModelTypeText(tr("Animated"));
            mModelInfoPanel->SetNumJointsText(QString::number(skelMdl->GetSkeleton()->GetNumBones()));
            mModelInfoPanel->SetNumAnimationsText(QString::number(numMotions));
        } else {
            mModelInfoPanel->SetModelTypeText(mdl->IsSoft() ? tr("Soft") : tr("Static"));
            mModelInfoPanel->SetNumJointsText("0");
            mModelInfoPanel->SetNumAnimationsText("0");
        }

        const size_t numVertices = mdl->GetVerticesCount();
        const size_t numTriangles = mdl->GetFacesCount();

        mModelInfoPanel->SetNumVerticesText(QString::number(numVertices));
        mModelInfoPanel->SetNumTrianglesText(QString::number(numTriangles));

        mModelInfoPanel->SetPlayStopButtonText(tr("Play"));

        //if (mDlgModelInfo) {
        //    mDlgModelInfo->SetModel(mdl);
        //}
    }
}

void MainWindow::ShowLevel(MyHandle file) {
    this->SwitchViewPanel(PanelType::Model);
    this->SwitchInfoPanel(PanelType::Sound);

    MetroLevel* level = new MetroLevel();
    if (level->LoadFromFileHandle(MetroFSPath(file))) {
        mRenderPanel->SetLevel(level);
    } else {
        MySafeDelete(level);
    }
}

void MainWindow::ShowLocalization(MyHandle file) {
    this->SwitchViewPanel(PanelType::Localization);
    this->SwitchInfoPanel(PanelType::Localization);

    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(MetroFSPath(file));
    if (stream) {
        MetroLocalization loc;
        if (loc.LoadFromData(stream)) {
            mLocalizationPanel->clear();
            mLocalizationPanel->setColumnCount(2);
            mLocalizationPanel->setHorizontalHeaderLabels(QStringList({ tr("Key"), tr("Value") }));

            const size_t numStrings = loc.GetNumStrings();
            mLocalizationPanel->setRowCount(scast<int>(numStrings));

            for (size_t i = 0; i < numStrings; ++i) {
                mLocalizationPanel->setItem(scast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(loc.GetKey(i))));
                mLocalizationPanel->setItem(scast<int>(i), 1, new QTableWidgetItem(QString::fromStdWString(loc.GetValue(i))));
            }
        }
    }
}

void MainWindow::ShowBin(MyHandle file, size_t subIdx) {
    this->SwitchViewPanel(PanelType::Empty);
    this->SwitchInfoPanel(PanelType::Empty);

    if (subIdx == kInvalidValue) {
        return;
    }

    const MetroConfigsDatabase& mcfgdb = MetroContext::Get().GetConfigsDB();
    auto stream = mcfgdb.GetFileStream(mcfgdb.GetFileByIdx(subIdx).nameCRC);
    if (stream.Good()) {
            MetroBinArchive bin(kEmptyString, stream, 0);
            StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();
            if (reader) {
                MetroReflectionStream* scriptReader = reader->OpenSection("script");
                if (scriptReader) {
                    MetroScript script;
                    *scriptReader >> script;
                    reader->CloseSection(scriptReader);

                    auto newScene = CreateScriptScene(script);
                    mVisualScriptPanel->setScene(newScene.get());
                    mCurScriptScene = std::move(newScene);

                    this->SwitchViewPanel(PanelType::VisualScript);
                    this->SwitchInfoPanel(PanelType::VisualScript);
                }
            }
    }
}

void MainWindow::SwitchViewPanel(const PanelType t) {
    mImagePanel->hide();
    mRenderPanel->hide();
    mLocalizationPanel->clear();
    mLocalizationPanel->hide();
    //mSoundPanel->Hide();
    mVisualScriptPanel->hide();

    switch (t) {
        case PanelType::Texture: {
            mImagePanel->show();
        } break;

        case PanelType::Model: {
            mRenderPanel->show();
        } break;

        case PanelType::Sound: {
            //mSoundPanel->Show();
        } break;

        case PanelType::Localization: {
            mLocalizationPanel->show();
        } break;

        case PanelType::VisualScript: {
            mVisualScriptPanel->show();
        } break;

        case PanelType::Empty: {
        } break;
    }
}

void MainWindow::SwitchInfoPanel(const PanelType t) {
    mModelInfoPanel->hide();
    mImageInfoPanel->hide();

    switch (t) {
        case PanelType::Texture: {
            mImageInfoPanel->show();
        } break;

        case PanelType::Model: {
            mModelInfoPanel->show();
        } break;

        case PanelType::Sound:
        case PanelType::Localization:
        case PanelType::VisualScript:
        case PanelType::Empty: {
        } break;
    }
}

// context menus
void MainWindow::ShowContextMenuFolder(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;

    QAction* extractFolder = menu.addAction(tr("Extract folder..."));
    QAction* extractFolderWithConversion = menu.addAction(tr("Extract folder with conversion..."));

    QAction* selectedAction = menu.exec(pos);
    if (selectedAction) {
        Q_UNUSED(extractFolder);
        this->OnExtractFolderClicked(selectedAction == extractFolderWithConversion);
    }
}

void MainWindow::ShowContextMenuTexture(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;

    QAction* saveAsDDS = menu.addAction(tr("Save as DDS..."));
    QAction* saveAsLegacyDDS = menu.addAction(tr("Save as legacy DDS..."));
    QAction* saveAsTGA = menu.addAction(tr("Save as TGA..."));
    QAction* saveAsPNG = menu.addAction(tr("Save as PNG..."));
    menu.addSeparator();

    //#NOTE_SK: if this is an albedo texture - enable whole set extraction option
    const MetroTexturesDatabase& mtxdb = MetroContext::Get().GetTexturesDB();
    QAction* saveSurfaceSet = mtxdb.IsAlbedo(mExtractionCtx.file) ? menu.addAction(tr("Save surface set...")) : nullptr;

    if (mtxdb.IsCubemap(mExtractionCtx.file)) {
        saveAsTGA->setVisible(false);
        saveAsPNG->setVisible(false);
    }

    bool shouldExtractTexture = false;

    const QAction* selectedAction = menu.exec(pos);

    mExtractionCtx.txUseBC3 = false;
    mExtractionCtx.txSaveAsDds = false;
    mExtractionCtx.txSaveAsTga = false;
    mExtractionCtx.txSaveAsPng = false;

    if (selectedAction == saveAsDDS) {
        mExtractionCtx.txSaveAsDds = true;
        mExtractionCtx.txUseBC3 = false;
        shouldExtractTexture = true;
    } else if (selectedAction == saveAsLegacyDDS) {
        mExtractionCtx.txSaveAsDds = true;
        mExtractionCtx.txUseBC3 = true;
        shouldExtractTexture = true;
    } else if (selectedAction == saveAsTGA) {
        mExtractionCtx.txSaveAsTga = true;
        shouldExtractTexture = true;
    } else if (selectedAction == saveAsPNG) {
        mExtractionCtx.txSaveAsPng = true;
        shouldExtractTexture = true;
    } else if (selectedAction == saveSurfaceSet) {
        MetroSurfaceDescription surface = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromFile(mExtractionCtx.file, false);

        this->EnsureExtractionOptions();
        mExtractionCtx.batch = false;
        mExtractionCtx.raw = false;

        shouldExtractTexture = false;

        this->ExtractSurfaceSet(mExtractionCtx, surface, fs::path());
    }

    if (shouldExtractTexture) {
        if (!this->ExtractTexture(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract texture!"));
        }
    }
}

void MainWindow::ShowContextMenuModel(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;

    QAction* saveAsOBJ = menu.addAction(tr("Save as OBJ..."));
    QAction* saveAsFBX = menu.addAction(tr("Save as FBX..."));

    bool shouldExtractModel = false;

    const QAction* selectedAction = menu.exec(pos);

    if (selectedAction == saveAsOBJ) {
        this->EnsureExtractionOptions();
        mExtractionCtx.mdlSaveAsObj = true;
        mExtractionCtx.mdlSaveAsFbx = false;
        shouldExtractModel = true;
    } else if (selectedAction == saveAsFBX) {
        this->EnsureExtractionOptions();
        mExtractionCtx.mdlSaveAsObj = false;
        mExtractionCtx.mdlSaveAsFbx = true;
        shouldExtractModel = true;
    }

    if (shouldExtractModel) {
        mExtractionCtx.batch = false;
        if (!this->ExtractModel(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract model!"));
        }
    }
}

void MainWindow::ShowContextMenuSound(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;

    QAction* saveAsOGG = menu.addAction(tr("Save as OGG..."));
    QAction* saveAsWAV = menu.addAction(tr("Save as WAV..."));

    bool shouldExtractSound = false;

    const QAction* selectedAction = menu.exec(pos);

    if (selectedAction == saveAsOGG) {
        mExtractionCtx.sndSaveAsOgg = true;
        mExtractionCtx.sndSaveAsWav = false;
        shouldExtractSound = true;
    } else if (selectedAction == saveAsWAV) {
        mExtractionCtx.sndSaveAsOgg = false;
        mExtractionCtx.sndSaveAsWav = true;
        shouldExtractSound = true;
    }

    if (shouldExtractSound) {
        if (!this->ExtractSound(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract sound!"));
        }
    }
}

void MainWindow::ShowContextMenuLocalization(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;
    menu.addAction(tr("Save as Excel 2003 XML..."));

    if (menu.exec(pos)) {
        if (!this->ExtractLocalization(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract localization!"));
        }
    }
}

void MainWindow::ShowContextMenuBin(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;

    QAction* extractRootFile = menu.addAction(tr("Extract root file..."));
    QAction* extractThisFile = menu.addAction(tr("Extract this file..."));

    bool shouldExtractFile = false;

    const QAction* selectedAction = menu.exec(pos);

    if (selectedAction == extractRootFile) {
        mExtractionCtx.customOffset = kInvalidValue;
        mExtractionCtx.customLength = kInvalidValue;
        mExtractionCtx.customFileName = "";
        shouldExtractFile = true;
    } else if (selectedAction == extractThisFile) {
        shouldExtractFile = true;
    }

    if (shouldExtractFile) {
        if (!this->ExtractFile(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract file!"));
        }
    }
}

void MainWindow::ShowContextMenuConfigBin(QTreeWidgetItem* /*node*/, const QPoint& pos, const bool enableModified) {
    QMenu menu;

    QAction* extract = menu.addAction(tr("Extract..."));
    menu.addSeparator();
    QAction* saveModified = menu.addAction(tr("Save modified..."));
    saveModified->setEnabled(enableModified);

    const QAction* selectedAction = menu.exec(pos);

    if (selectedAction == extract) {
        mExtractionCtx.customOffset = kInvalidValue;
        mExtractionCtx.customLength = kInvalidValue;
        mExtractionCtx.customFileName = "";

        if (!this->ExtractFile(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract file!"));
        }
    } else if (selectedAction == saveModified) {
        const MemStream& stream = MetroContext::Get().GetConfigsDB().GetDataStream();
        if (stream.Good()) {
            fs::path resultPath;

            QString saveName = QFileDialog::getSaveFileName(this, tr("Save Configs database..."), QString("config.bin"), tr("Bin file (*.bin)"));

            if (saveName.length() > 3) {
                const bool result = OSWriteFile(saveName.toStdWString(), stream.Data(), stream.Length()) == stream.Length();

                if (!result) {
                    QMessageBox::critical(this, this->windowTitle(), tr("Failed to save Configs Database!"));
                } else {
                    QMessageBox::information(this, this->windowTitle(), tr("Configs Database was successfully saved!"));
                }
            }
        }
    }
}

void MainWindow::ShowContextMenuRAW(QTreeWidgetItem* /*node*/, const QPoint& pos) {
    QMenu menu;
    menu.addAction(tr("Extract file..."));

    if (menu.exec(pos)) {
        if (!this->ExtractFile(mExtractionCtx, fs::path())) {
            QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract file!"));
        }
    }
}


// extraction
//  folder
void MainWindow::OnExtractFolderClicked(bool withConversion) {
    fs::path folderPath = QFileDialog::getExistingDirectory(this, tr("Choose output directory...")).toStdWString();
    if (!folderPath.empty()) {
        mExtractionCtx.batch = true;
        mExtractionCtx.raw = !withConversion;
        mExtractionCtx.numFilesTotal = MetroContext::Get().GetFilesystem().CountFilesInFolder(mExtractionCtx.file, true);
        mExtractionCtx.progress = 0;

        HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IProgressDialog), (void**)&mExtractionProgressDlg);
        if (SUCCEEDED(hr)) {
            mExtractionProgressDlg->SetTitle(L"Extracting files...");
            mExtractionProgressDlg->SetLine(0, L"Please wait while your files are being extracted...", FALSE, nullptr);
            mExtractionProgressDlg->StartProgressDialog(rcast<HWND>(this->winId()), nullptr,
                PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE,
                nullptr);
        }

        if (mExtractionThread.joinable()) {
            mExtractionThread.join();
        }
        mExtractionThread = std::thread(&MainWindow::ExtractionProcessFunc, this, folderPath);
    }
}


// extraction helpers
void MainWindow::EnsureExtractionOptions() {
    MEXSettings& s = MEXSettings::Get();

    if (s.extraction.askEveryTime) {
        SettingsDlg dlg(this);
        dlg.setWindowIcon(this->windowIcon());
        dlg.exec();
    }

    // models
    mExtractionCtx.mdlSaveAsObj = (s.extraction.modelFormat == MEXSettings::Extraction::MdlFormat::Obj);
    mExtractionCtx.mdlSaveAsFbx = (s.extraction.modelFormat == MEXSettings::Extraction::MdlFormat::Fbx);
    mExtractionCtx.mdlSaveWithAnims = s.extraction.modelSaveWithAnims;
    mExtractionCtx.mdlAnimsSeparate = s.extraction.modelAnimsSeparate;
    mExtractionCtx.mdlSaveWithTextures = s.extraction.modelSaveWithTextures;
    mExtractionCtx.mdlExcludeCollision = s.extraction.modelExcludeCollision;
    mExtractionCtx.mdlSaveLods = s.extraction.modelSaveLods;
    // textures
    mExtractionCtx.txSaveAsDds = (s.extraction.textureFormat == MEXSettings::Extraction::TexFormat::Dds || s.extraction.textureFormat == MEXSettings::Extraction::TexFormat::LegacyDds);
    mExtractionCtx.txUseBC3 = (s.extraction.textureFormat == MEXSettings::Extraction::TexFormat::LegacyDds);
    mExtractionCtx.txSaveAsTga = (s.extraction.textureFormat == MEXSettings::Extraction::TexFormat::Tga);
    mExtractionCtx.txSaveAsPng = (s.extraction.textureFormat == MEXSettings::Extraction::TexFormat::Png);
    // sounds
    mExtractionCtx.sndSaveAsOgg = (s.extraction.soundFormat == MEXSettings::Extraction::SndFormat::Ogg);
    mExtractionCtx.sndSaveAsWav = (s.extraction.soundFormat == MEXSettings::Extraction::SndFormat::Wav);
}

CharString MainWindow::DecideTextureExtension(const FileExtractionCtx& ctx) {
    CharString result = ctx.txSaveAsDds ? ".dds" : (ctx.txSaveAsTga ? ".tga" : ".png");
    return result;
}

CharString MainWindow::MakeFileOutputName(MyHandle file, const FileExtractionCtx& ctx) {
    CharString name = MetroContext::Get().GetFilesystem().GetName(MetroFSPath(file));

    switch (ctx.type) {
        case FileType::Texture: {
            const CharString::size_type dotPos = name.find_last_of('.');
            const size_t replaceLen = name.size() - dotPos;
            name = name.replace(dotPos, replaceLen, this->DecideTextureExtension(ctx));
        } break;

        case FileType::Model: {
            const CharString::size_type dotPos = name.find_last_of('.');
            const size_t replaceLen = name.size() - dotPos;

            if (ctx.mdlSaveAsObj) {
                name = name.replace(dotPos, replaceLen, ".obj");
            } else {
                name = name.replace(dotPos, replaceLen, ".fbx");
            }
        } break;

        case FileType::Sound: {
            if (ctx.sndSaveAsOgg) {
                name[name.size() - 3] = 'o';
                name[name.size() - 2] = 'g';
                name[name.size() - 1] = 'g';
            } else {
                name[name.size() - 3] = 'w';
                name[name.size() - 2] = 'a';
                name[name.size() - 1] = 'v';
            }
        } break;

        case FileType::Localization: {
            name[name.size() - 3] = 'x';
            name[name.size() - 2] = 'm';
            name[name.size() - 1] = 'l';
        } break;

        case FileType::Level: {
            name[name.size() - 3] = 'f';
            name[name.size() - 2] = 'b';
            name[name.size() - 1] = 'x';
        } break;

        default:
            break;
    }

    return name;
}

void MainWindow::TextureSaveHelper(const fs::path& folderPath, const FileExtractionCtx& ctx, const CharString& name) {
    CharString textureName = CharString("content\\textures\\") + name;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

    CharString textureNameSrc = textureName + ".4096";
    MetroFSPath textureHandle = mfs.FindFile(textureNameSrc);
    if (!textureHandle.IsValid()) {
        textureNameSrc = textureName + ".2048";
        textureHandle = mfs.FindFile(textureNameSrc);
    }
    if (!textureHandle.IsValid()) {
        textureNameSrc = textureName + ".1024";
        textureHandle = mfs.FindFile(textureNameSrc);
    }
    if (!textureHandle.IsValid()) {
        textureNameSrc = textureName + ".512";
        textureHandle = mfs.FindFile(textureNameSrc);
    }

    if (!textureHandle.IsValid()) {
        // last try - Redux .bin
        textureNameSrc = textureName + ".bin";
        textureHandle = mfs.FindFile(textureNameSrc);
    }

    if (textureHandle.IsValid()) {
        FileExtractionCtx tmpCtx = ctx;
        tmpCtx.type = FileType::Texture;
        tmpCtx.file = textureHandle;

        CharString outName = this->MakeFileOutputName(textureHandle.fileHandle, tmpCtx);
        this->ExtractTexture(tmpCtx, folderPath / outName);
    }
}

bool MainWindow::ExtractFile(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    const CharString& fileName = MetroContext::Get().GetFilesystem().GetName(ctx.file);
    QString name = QString::fromStdString(ctx.customFileName.empty() ? fileName : ctx.customFileName);

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        QString saveName = QFileDialog::getSaveFileName(this, tr("Save file..."), name, tr("All files (*.*)"));

        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    if (!resultPath.empty()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(ctx.file);
        if (stream) {
            const bool hasCustomLength = ctx.customLength != kInvalidValue;
            const bool hasCustomOffset = ctx.customOffset != kInvalidValue;

            size_t lengthToWrite = hasCustomLength ? ctx.customLength : stream.Remains();

            if (hasCustomOffset) {
                stream.SetCursor(ctx.customOffset);

                if (!hasCustomLength) {
                    lengthToWrite = stream.Length() - ctx.customOffset;
                }
            }

            const size_t bytesWritten = OSWriteFile(resultPath, stream.GetDataAtCursor(), lengthToWrite);
            result = (bytesWritten == lengthToWrite);
        }
    }

    return result;
}

bool MainWindow::ExtractTexture(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        QString title;
        QString filter;
        if (ctx.txSaveAsDds) {
            title = tr("Save DDS texture...");
            filter = tr("DirectDraw Surface (*.dds)");
        } else if (ctx.txSaveAsTga) {
            title = tr("Save TGA texture...");
            filter = tr("Targa images (*.tga)");
        } else {
            title = tr("Save PNG texture...");
            filter = tr("PNG images (*.png)");
        }

        CharString nameWithExt = this->MakeFileOutputName(ctx.file.fileHandle, ctx);

        QString saveName = QFileDialog::getSaveFileName(this, title, QString::fromStdString(nameWithExt), filter);

        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    if (!resultPath.empty()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(ctx.file);
        if (stream) {
            const CharString& fileName = MetroContext::Get().GetFilesystem().GetName(ctx.file);

            MetroTexture texture;
            if (texture.LoadFromData(stream, fileName)) {
                if (ctx.txSaveAsDds) {
                    if (ctx.txUseBC3) {
                        result = texture.SaveAsLegacyDDS(resultPath);
                    } else {
                        result = texture.SaveAsDDS(resultPath);
                    }
                } else if (ctx.txSaveAsTga) {
                    result = texture.SaveAsTGA(resultPath);
                } else {
                    result = texture.SaveAsPNG(resultPath);
                }
            }
        }
    }

    return result;
}

bool MainWindow::ExtractSurfaceSet(const FileExtractionCtx& ctx, const MetroSurfaceDescription& surface, const fs::path& outFolder) {
    bool result = false;

    fs::path folderPath = outFolder;
    if (folderPath.empty()) {
        folderPath = QFileDialog::getExistingDirectory(this, tr("Choose output directory...")).toStdWString();
    }

    if (!folderPath.empty()) {
        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

        FileExtractionCtx setCtx = ctx;
        setCtx.type = FileType::Texture;

#define EXTRACT_SURFACE_TEXTURE(tex_name)                                                       \
            if (!surface.tex_name##Paths.empty()) {                                             \
                const MetroFSPath& file = mfs.FindFile(surface.tex_name##Paths.front());        \
                if (file.IsValid()) {                                                           \
                    setCtx.file = file;                                                         \
                    CharString nameWithExt = this->MakeFileOutputName(file.fileHandle, setCtx); \
                    result = this->ExtractTexture(setCtx, folderPath / nameWithExt) && result;  \
                }                                                                               \
            }

        EXTRACT_SURFACE_TEXTURE(albedo);
        EXTRACT_SURFACE_TEXTURE(bump);
        EXTRACT_SURFACE_TEXTURE(normalmap);
        EXTRACT_SURFACE_TEXTURE(detail);

#undef EXTRACT_SURFACE_TEXTURE
    }

    return result;
}

bool MainWindow::ExtractModel(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        QString title;
        QString filter;
        if (ctx.mdlSaveAsObj) {
            title = tr("Save OBJ model...");
            filter = tr("OBJ model (*.obj)");
        } else {
            title = tr("Save FBX model...");
            filter = tr("FBX model (*.fbx)");
        }

        CharString nameWithExt = this->MakeFileOutputName(ctx.file.fileHandle, ctx);

        QString saveName = QFileDialog::getSaveFileName(this, title, QString::fromStdString(nameWithExt), filter);

        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    const MEXSettings& settings = MEXSettings::Get();

    if (!resultPath.empty()) {
        RefPtr<MetroModelBase> mdl = MetroModelFactory::CreateModelFromFile(ctx.file, MetroModelLoadParams::LoadEverything);
        if (mdl) {
            CharString texExt = this->DecideTextureExtension(ctx);

            if (ctx.mdlSaveAsObj) {
                ExporterOBJ expObj;
                expObj.SetExcludeCollision(ctx.mdlExcludeCollision);
                expObj.SetTexturesExtension(texExt);
                expObj.ExportModel(*mdl, resultPath);
                if (ctx.mdlSaveLods && (mdl->GetModelType() == MetroModelType::Hierarchy || mdl->GetModelType() == MetroModelType::Hierarchy2)) {
                    //RefPtr<MetroModelHierarchy> hierarchyMdl = SCastRefPtr<MetroModelHierarchy>(mdl);
                    //MetroModel* lod1 = mdl.GetLodModel(0);
                    //MetroModel* lod2 = mdl.GetLodModel(1);

                    //if (lod1) {
                    //    fs::path lodPath = resultPath;
                    //    lodPath.replace_extension("_lod1.obj");
                    //    expObj.ExportModel(*lod1, lodPath);
                    //}
                    //if (lod2) {
                    //    fs::path lodPath = resultPath;
                    //    lodPath.replace_extension("_lod2.obj");
                    //    expObj.ExportModel(*lod2, lodPath);
                    //}
                }
            } else {
                ExporterFBX expFbx;
                expFbx.SetExportMesh(true);
                expFbx.SetExportSkeleton(true);
                if (ctx.mdlExcludeCollision) {
                    expFbx.SetExcludeCollision(true);
                }
                if (settings.extraction.modelSaveWithAnims && !settings.extraction.modelAnimsSeparate) {
                    expFbx.SetExportAnimation(true);
                }

                expFbx.SetTexturesExtension(texExt);
                expFbx.ExportModel(*mdl, resultPath);

                if (ctx.mdlSaveLods) {
                    //MetroModel* lod1 = mdl.GetLodModel(0);
                    //MetroModel* lod2 = mdl.GetLodModel(1);

                    //expFbx.SetExportAnimation(false);

                    //if (lod1) {
                    //    fs::path lodPath = resultPath;
                    //    lodPath.replace_extension("_lod1.fbx");
                    //    expFbx.ExportModel(*lod1, lodPath);
                    //}
                    //if (lod2) {
                    //    fs::path lodPath = resultPath;
                    //    lodPath.replace_extension("_lod2.fbx");
                    //    expFbx.ExportModel(*lod2, lodPath);
                    //}
                }

                if (settings.extraction.modelSaveWithAnims && settings.extraction.modelAnimsSeparate && mdl->IsSkeleton()) {
                    RefPtr<MetroModelSkeleton> skelMdl = SCastRefPtr<MetroModelSkeleton>(mdl);
                    RefPtr<MetroSkeleton> skeleton = skelMdl->GetSkeleton();
                    if (skeleton) {
                        expFbx.SetExportMesh(false);
                        expFbx.SetExportAnimation(true);

                        fs::path modelBasePath = resultPath.parent_path() / resultPath.stem();
                        for (size_t motionIdx = 0; motionIdx != skeleton->GetNumMotions(); ++motionIdx) {
                            RefPtr<MetroMotion> motion = skeleton->GetMotion(motionIdx);
                            fs::path animPath = modelBasePath.native() + fs::path("@" + motion->GetName()).native() + L".fbx";
                            expFbx.SetExportMotionIdx(motionIdx);
                            expFbx.ExportModel(*mdl, animPath);
                        }
                    }
                }
            }

            if (!ctx.batch && ctx.mdlSaveWithTextures) {
                fs::path folderPath = resultPath.parent_path();
                MyArray<MetroModelGeomData> gds;
                mdl->CollectGeomData(gds);
                for (auto& gd : gds) {
                    CharString textureName(gd.texture);

                    if (settings.extraction.modelSaveSurfaceSet) {
                        MetroSurfaceDescription surface = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromName(textureName, false);
                        this->ExtractSurfaceSet(ctx, surface, folderPath);
                    } else {
                        const CharString& sourceName = MetroContext::Get().GetTexturesDB().GetSourceName(textureName);
                        this->TextureSaveHelper(folderPath, ctx, sourceName);
                    }
                }
            }

            result = true;
        }
    }

    return result;
}

bool MainWindow::ExtractMotion(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    RefPtr<MetroModelBase> model = mRenderPanel->GetModel();
    const int motionIdx = mModelInfoPanel->GetSelectedMotionIdx();

    if (!model || !model->IsSkeleton() || motionIdx < 0) {
        return false;
    }

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        RefPtr<MetroModelSkeleton> skelModel = SCastRefPtr<MetroModelSkeleton>(model);
        RefPtr<MetroSkeleton> skeleton = skelModel->GetSkeleton();

        CharString motionName = skeleton->GetMotionName(scast<size_t>(motionIdx));

        QString fileName = QString::fromStdString(motionName).append(".fbx");
        QString saveName = QFileDialog::getSaveFileName(this, tr("Save FBX animation..."), fileName, tr("FBX animation (*.fbx)"));
        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    if (!resultPath.empty()) {
        ExporterFBX expFbx;
        expFbx.SetExportMesh(false);
        expFbx.SetExportSkeleton(true);
        expFbx.SetExportAnimation(true);
        expFbx.SetExportMotionIdx(scast<size_t>(motionIdx));

        result = expFbx.ExportModel(*model, resultPath);
    }

    return result;
}

bool MainWindow::ExtractSound(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        QString title;
        QString filter;
        if (ctx.sndSaveAsOgg) {
            title = tr("Save Ogg sound...");
            filter = tr("Ogg Vorbis (*.ogg)");
        } else {
            title = tr("Save WAV sound...");
            filter = tr("Wave sounds (*.wav)");
        }

        CharString nameWithExt = this->MakeFileOutputName(ctx.file.fileHandle, ctx);

        QString saveName = QFileDialog::getSaveFileName(this, title, QString::fromStdString(nameWithExt), filter);

        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    if (!resultPath.empty()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(ctx.file);
        if (stream) {
            MetroSound sound;
            if (sound.LoadFromData(stream)) {
                if (ctx.sndSaveAsOgg) {
                    result = sound.SaveAsOGG(resultPath);
                } else {
                    result = sound.SaveAsWAV(resultPath);
                }
            }
        }
    }

    return result;
}

bool MainWindow::ExtractLocalization(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    fs::path resultPath = outPath;
    if (resultPath.empty()) {
        CharString nameWithExt = this->MakeFileOutputName(ctx.file.fileHandle, ctx);

        QString saveName = QFileDialog::getSaveFileName(this, tr("Save Excel 2003 XML..."), QString::fromStdString(nameWithExt), tr("Excel 2003 XML (*.xml)"));

        if (saveName.length() > 3) {
            resultPath = saveName.toStdWString();
        } else {
            return true;
        }
    }

    if (!resultPath.empty()) {
        MemStream stream = MetroContext::Get().GetFilesystem().OpenFileStream(ctx.file);
        if (stream) {
            MetroLocalization loc;
            if (loc.LoadFromData(stream)) {
                result = loc.SaveToExcel2003(resultPath);
            }
        }
    }

    return result;
}

bool MainWindow::ExtractFolderComplete(const FileExtractionCtx& ctx, const fs::path& outPath) {
    bool result = false;

    const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    const CharString& folderName = mfs.GetName(ctx.file);

    fs::path curPath = outPath / folderName;
    fs::create_directories(curPath);

    FileExtractionCtx tmpCtx = ctx;
    for (MyHandle child = mfs.GetFirstChild(ctx.file.fileHandle); child != kInvalidHandle; child = mfs.GetNextChild(child)) {
        tmpCtx.file = MetroFSPath(child);
        tmpCtx.type = DetectFileType(tmpCtx.file);

        const bool isFolder = mfs.IsFolder(tmpCtx.file);
        if (isFolder) {
            this->ExtractFolderComplete(tmpCtx, curPath);
        } else {
            if (ctx.raw) {
                const CharString& childName = mfs.GetName(tmpCtx.file);
                fs::path filePath = curPath / childName;
                this->ExtractFile(tmpCtx, filePath);
            } else {
                fs::path filePath = curPath / this->MakeFileOutputName(child, tmpCtx);
                switch (tmpCtx.type) {
                    case FileType::Texture: {
                        this->ExtractTexture(tmpCtx, filePath);
                    } break;

                    case FileType::Model: {
                        this->ExtractModel(tmpCtx, filePath);
                    } break;

                    case FileType::Sound: {
                        this->ExtractSound(tmpCtx, filePath);
                    } break;

                    case FileType::Localization: {
                        this->ExtractLocalization(tmpCtx, filePath);
                    } break;

                    default: {
                        this->ExtractFile(tmpCtx, filePath);
                    } break;
                }
            }

            mExtractionCtx.progress++;
            if (mExtractionProgressDlg) {
                mExtractionProgressDlg->SetProgress64(mExtractionCtx.progress, mExtractionCtx.numFilesTotal);

                //If cancelled - just exit the loop
                if (mExtractionProgressDlg->HasUserCancelled() == TRUE) {
                    break;
                }
            }
        }
    }

    result = true;

    return result;
}

void MainWindow::ExtractionProcessFunc(const fs::path& folderPath) {
    this->EnsureExtractionOptions();
    this->ExtractFolderComplete(mExtractionCtx, folderPath);

    if (mExtractionProgressDlg) {
        mExtractionProgressDlg->StopProgressDialog();
        MySafeRelease(mExtractionProgressDlg);
    }
}

// property panels
// model props
void MainWindow::OnModelInfoMotionsSelectedIndexChanged(int selection) {
    if (selection >= 0) {
        mRenderPanel->SwitchMotion(scast<size_t>(selection));
    }
}

void MainWindow::OnModelInfoLodsSelectedIndexChanged(int selection) {
    if (selection >= 0) {
        mRenderPanel->SetLod(scast<size_t>(selection));
    }
}

void MainWindow::OnModelInfoPlayStopAnimClicked() {
    mRenderPanel->PlayAnim(!mRenderPanel->IsPlayingAnim());
    mModelInfoPanel->SetPlayStopButtonText(mRenderPanel->IsPlayingAnim() ? tr("Stop") : tr("Play"));
}

void MainWindow::OnModelInfoInfoClicked() {
    //if (!mDlgModelInfo) {
    //    mDlgModelInfo = gcnew MetroEX::DlgModelInfo();
    //    mDlgModelInfo->Closed += gcnew System::EventHandler(this, &MetroEX::MainWindowImpl::OnDlgModelInfo_Closed);

    //    mDlgModelInfo->Icon = this->Icon;
    //    mDlgModelInfo->SetModel(mRenderPanel->GetModel());
    //    mDlgModelInfo->Show();
    //}
}

void MainWindow::OnModelInfoExportMotionClicked() {
    if (!this->ExtractMotion(mExtractionCtx, fs::path())) {
        QMessageBox::critical(this, this->windowTitle(), tr("Failed to extract motion!"));
    }
}

void MainWindow::AddArchiveToHistory(const WideString& path) {
    MEXSettings& settings = MEXSettings::Get();

    AddPathToHistory(path, settings.openHistory.archives);
    mToolbar->SetOpenArchiveHistory(settings.openHistory.archives);
}

void MainWindow::AddFolderToHistory(const WideString& path) {
    MEXSettings& settings = MEXSettings::Get();

    AddPathToHistory(path, settings.openHistory.folders);
    mToolbar->SetOpenGameFolderHistory(settings.openHistory.folders);
}

void MainWindow::AddPathToHistory(const WideString& path, WStringArray& history) {
    const auto iterator = std::find(history.begin(), history.end(), path);
    if (iterator != history.end()) {
        history.erase(iterator);
    }

    while (history.size() >= 10)
    {
        history.erase(history.begin());
    }

    history.push_back(path);

    MEXSettings& settings = MEXSettings::Get();
    settings.Save();
}

WideString MainWindow::GetLastArchivePathFromHistory() const {
    MEXSettings& settings = MEXSettings::Get();

    for (long i = settings.openHistory.archives.size() - 1; i >= 0; --i) {
        auto path = std::filesystem::path(settings.openHistory.archives[i]).parent_path();

        if (exists(path)) {
            return path.generic_wstring();
        }
    }

    return WideString();
}
