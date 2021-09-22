#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "addtexturewnd.h"
#include "imagepanel.h"

#include "MetroTextureInfoData.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>

#include "metro/MetroTexturesDatabase.h"
#include "metro/MetroTexture.h"
#include "metro/MetroContext.h"

enum class TextureType : int {
    Diffuse     = 0,
    Detail      = 1,
    Cubemap     = 2,
    Cubemap_hdr = 3,
    Terrain     = 4,
    Bump        = 5,
    Diffuse_va  = 6
};

static void TextureInfoProps2033ToTextureCommonInfo(MetroTextureInfoData* props, MetroTextureInfoCommon& info) {
    info.type        = props->type;
    info.animated    = props->animated;
    info.fmt         = props->fmt;
    info.r_width     = props->width;
    info.r_height    = props->height;
    info.name        = props->name.toStdString();
    info.bump_name   = props->bump_name.toStdString();
    info.bump_height = props->bump_height;
    info.parr_height = props->parr_height;
    info.det_name    = props->det_name.toStdString();
    info.det_u_scale = props->det_u_scale;
    info.det_v_scale = props->det_v_scale;
    info.det_int     = props->det_int;
    info.mip_enabled = props->mip_enabled;
    info.streamable  = props->streamable;
    info.priority    = props->priority;
    info.avg_color   = (props->avg_color.red() << 24) | (props->avg_color.green() << 16) | (props->avg_color.blue() << 8);
}

void TextureCommonInfoToTextureInfoProps2033(const MetroTextureInfoCommon& info, MetroTextureInfoData* props) {
    props->type         = info.type;
    props->animated     = info.animated;
    props->fmt          = info.fmt;
    props->width        = info.r_width;
    props->height       = info.r_height;
    props->name         = QString::fromStdString(info.name);
    props->bump_name    = QString::fromStdString(info.bump_name);
    props->bump_height  = info.bump_height;
    props->parr_height  = info.parr_height;
    props->det_name     = QString::fromStdString(info.det_name);
    props->det_u_scale  = info.det_u_scale;
    props->det_v_scale  = info.det_v_scale;
    props->det_int      = info.det_int;
    props->mip_enabled  = info.mip_enabled;
    props->streamable   = info.streamable;
    props->priority     = info.priority;
    props->avg_color    = QColor::fromRgb((info.avg_color >> 24) & 255, (info.avg_color >> 16) & 255, (info.avg_color >> 8) & 255);
}


static bool LoadMetroTexture(const fs::path& texturesFolder, const MetroTextureInfoCommon& info, MetroTexture& texture) {
    fs::path fullPath = texturesFolder / info.name;
    if (!info.streamable) {
        fullPath += ".dds";
    } else {
        if (info.r_width == 2048) {
            fullPath += ".2048";
        } else if (info.r_width == 1024) {
            fullPath += ".1024";
        } else {
            fullPath += ".512";
        }
    }

    MemStream stream = OSReadFile(fullPath);
    if (stream) {
        return texture.LoadFromData(stream, fullPath.filename().string());
    } else {
        return false;
    }
}

static bool LoadMetroTextureFromExisting(const fs::path& path, MetroTexture& texture) {
    bool result = false;

    fs::path largestMip = path;

    WideString ext = path.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    if (ext != L".dds") {
        largestMip.replace_extension(L".2048");
        if (!OSPathExists(largestMip)) {
            largestMip.replace_extension(L".1024");
            if (!OSPathExists(largestMip)) {
                largestMip.replace_extension(L".512");
                if (!OSPathExists(largestMip)) {
                    largestMip.clear();
                }
            }
        }
    }

    if (!largestMip.empty()) {
        MemStream stream = OSReadFile(largestMip);
        if (stream) {
            result = texture.LoadFromData(stream, largestMip.filename().string());
        }
    }

    return result;
}


static const QString kLastOpenPath("LastOpenPath");
static const QString kLastSavePath("LastSavePath");



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mSearchTimer{}
    , mPropertyBrowser(new ObjectPropertyBrowser(this))
    , mImagePanel(new ImagePanel(this))
    , mTexturesDB{}
    , mTexturesFolder{}
    , mTextureInfoProps{}
{
    ui->setupUi(this);

    QHBoxLayout* propBrowserLayout = new QHBoxLayout();
    propBrowserLayout->setContentsMargins(0, 0, 0, 0);
    propBrowserLayout->setSpacing(0);
    ui->pnlPropertyGrid->setLayout(propBrowserLayout);
    propBrowserLayout->addWidget(mPropertyBrowser);
    connect(mPropertyBrowser, SIGNAL(objectPropertyChanged()), this, SLOT(onPropertyBrowserObjectPropertyChanged()));

    QHBoxLayout* imagePanelLayout = new QHBoxLayout();
    imagePanelLayout->setContentsMargins(0, 0, 0, 0);
    imagePanelLayout->setSpacing(0);
    ui->pnlImageView->setLayout(imagePanelLayout);
    mImagePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imagePanelLayout->addWidget(mImagePanel);


    //#NOTE_SK: Some hacky-hacky bullshit to make splitters to be approx. where we want them to be
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 8);
    ui->splitter_2->setStretchFactor(0, 6);
    ui->splitter_2->setStretchFactor(1, 1);


    mImagePanel->ShowTransparency(true);

    MetroContext::Get().SetGameVersion(MetroGameVersion::OG2033);

    mSearchTimer = MakeStrongPtr<QTimer>();
    mSearchTimer->setInterval(1000); // 1 second
    connect(mSearchTimer.get(), SIGNAL(timeout()), this, SLOT(onSearchTimerTick()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::FillTexturesList(const QString& subText) {
    ui->lstTextures->clear();

    const size_t numTextures = mTexturesDB->GetNumTextures();
    for (size_t i = 0; i < numTextures; ++i) {
        QString textureName = QString::fromStdString(mTexturesDB->GetTextureNameByIdx(i));

        if (subText.isEmpty() || textureName.contains(subText)) {
            QListWidgetItem* newItem = new QListWidgetItem(textureName);
            newItem->setData(Qt::UserRole, QVariant(scast<int>(i)));
            ui->lstTextures->addItem(newItem);
        }
    }
}

void MainWindow::AddOrReplaceTexture(bool replaceCurrent) {
    if (mTexturesDB) {
        AddTextureWnd wnd;
        wnd.setWindowIcon(this->windowIcon());

        if (QDialog::Accepted == wnd.exec()) {
            MetroTexture texture;
            bool loaded = false;

            const bool useExistingTexture = wnd.IsUseExistingTexture();
            fs::path outputPath = wnd.GetOutputPath();

            MyArray<fs::path> srcPaths = wnd.GetAddedTextures();
            for (const fs::path& sourcePath : srcPaths) {
                if (useExistingTexture) {
                    loaded = LoadMetroTextureFromExisting(sourcePath, texture);
                } else {
                    loaded = texture.LoadFromFile(sourcePath);
                }

                if (!loaded) {
                    QMessageBox::critical(this, this->windowTitle(), tr("Failed to load texture!"));
                } else {
                    if (!IsPowerOfTwo(texture.GetWidth()) || !IsPowerOfTwo(texture.GetHeight())) {
                        QMessageBox::critical(this, this->windowTitle(), tr("Texture size must be power of two!\nAcceptable sizes are: 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048"));
                        return;
                    }
                    if (texture.GetWidth() != texture.GetHeight()) {
                        QMessageBox::critical(this, this->windowTitle(), tr("Texture must be square for Metro 2033!\nPlease make sure Width == Height and retry"));
                        return;
                    }

                    bool isDDS = false;
                    CharString name;
                    bool success = true;
                    MetroTexture::PixelFormat fmt = texture.GetFormat();
                    if (useExistingTexture) {
                        fs::path relativePath = fs::relative(sourcePath, mTexturesFolder);
                        WideString ext = relativePath.extension().wstring();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                        isDDS = (ext == L".dds");
                        relativePath.replace_extension("");
                        name = relativePath.string();
                    } else {
                        fs::path outFolder = outputPath;
                        outFolder /= sourcePath.filename();
                        fs::path relativePath = fs::relative(outFolder, mTexturesFolder);
                        relativePath.replace_extension("");
                        name = relativePath.string();

                        outFolder.replace_extension("");
                        fmt = texture.HasAlpha() ? MetroTexture::PixelFormat::BC3 : MetroTexture::PixelFormat::BC1;
                        success = texture.SaveAsMetroTexture(outFolder, fmt);
                    }

                    if (!success) {
                        QMessageBox::critical(this, this->windowTitle(), tr("Failed to convert texture!"));
                    } else {
                        if (replaceCurrent) {
                            const size_t selectedIdx = scast<size_t>(this->GetSelectedTextureIdx());

                            MetroTextureInfoCommon info;
                            mTexturesDB->FillCommonInfoByIdx(selectedIdx, info);
                            info.fmt = scast<uint32_t>(fmt);
                            info.r_width = scast<uint32_t>(texture.GetWidth());
                            info.r_height = scast<uint32_t>(texture.GetHeight());
                            info.name = name;
                            info.mip_enabled = texture.GetNumMips() > 1;
                            info.streamable = !isDDS;
                            info.priority = !isDDS;
                            mTexturesDB->SetCommonInfoByIdx(selectedIdx, info);

                            this->on_lstTextures_currentRowChanged(selectedIdx);
                        } else {
                            MetroTextureInfoCommon info = {};
                            info.type = scast<int32_t>(TextureType::Diffuse);
                            info.fmt = scast<uint32_t>(fmt);
                            info.r_width = scast<uint32_t>(texture.GetWidth());
                            info.r_height = scast<uint32_t>(texture.GetHeight());
                            info.name = name;
                            info.parr_height = 2;
                            info.det_u_scale = 1.0f;
                            info.det_v_scale = 1.0f;
                            info.det_int = 1.0f;
                            info.mip_enabled = texture.GetNumMips() > 1;
                            info.streamable = !isDDS;
                            info.priority = !isDDS;

                            mTexturesDB->AddTexture(info);

                            const int newIdx = ui->lstTextures->count() - 1;
                            QListWidgetItem* newItem = new QListWidgetItem(QString::fromStdString(info.name));
                            newItem->setData(Qt::UserRole, QVariant(newIdx));
                            ui->lstTextures->addItem(newItem);
                            ui->lstTextures->setCurrentRow(newIdx);
                        }

                        // calculate texture avg colour
                        this->on_actionCalculate_texture_average_colour_triggered();
                    }
                }
            }
        }
    }
}

void MainWindow::on_lstTextures_currentRowChanged(int currentRow) {
    if (mTexturesDB && this->GetSelectedTextureIdx() >= 0) {
        MetroTextureInfoCommon info;
        mTexturesDB->FillCommonInfoByIdx(scast<size_t>(this->GetSelectedTextureIdx()), info);

        MetroTexture texture;
        if (LoadMetroTexture(mTexturesFolder, info, texture)) {
            BytesArray pixels;
            texture.GetRGBA(pixels);

            mImagePanel->SetImage(pixels.data(), texture.GetWidth(), texture.GetHeight());

            mPropertyBrowser->setActiveObject(nullptr);
            mTextureInfoProps = MakeStrongPtr<MetroTextureInfoData>();
            TextureCommonInfoToTextureInfoProps2033(info, mTextureInfoProps.get());
            mPropertyBrowser->setActiveObject(mTextureInfoProps.get());
        } else {
            mImagePanel->SetImage(nullptr, 0, 0);
            mPropertyBrowser->setActiveObject(nullptr);
        }
    }
}

void MainWindow::on_actionOpen_textures_bin_triggered() {
    QSettings registry;
    QString lastOpenDir = registry.value(kLastOpenPath).toString();

    QString openName = QFileDialog::getOpenFileName(this, tr("Open Metro 2033 textures database..."), lastOpenDir, tr("Metro bin file (*.bin)"));
    if (!openName.isEmpty()) {
        fs::path binPath = openName.toStdWString();
        fs::path folderPath = fs::absolute(binPath.parent_path());

        mTexturesDB = MakeStrongPtr<MetroTexturesDatabase>();
        if (mTexturesDB->Initialize(MetroGameVersion::OG2033, binPath)) {
            mTexturesFolder = folderPath;

            this->FillTexturesList();
        }

        lastOpenDir = QString::fromStdWString(folderPath.wstring());
        registry.setValue(kLastOpenPath, lastOpenDir);
    }
}

void MainWindow::on_actionSave_textures_bin_triggered() {
    if (mTexturesDB) {
        QSettings registry;
        QString lastSavePath = registry.value(kLastSavePath).toString();
        if (lastSavePath.isEmpty()) {
            lastSavePath = "textures.bin";
        }

        QString saveName = QFileDialog::getSaveFileName(this, tr("Save Metro 2033 textures database..."), lastSavePath, tr("Metro bin file (*.bin)"));
        if (!saveName.isEmpty()) {
            fs::path savePath = fs::absolute(saveName.toStdWString());
            mTexturesDB->SaveBin(savePath);

            lastSavePath = QString::fromStdWString(savePath.wstring());
            registry.setValue(kLastSavePath, lastSavePath);
        }
    }
}

void MainWindow::on_actionAdd_texture_triggered() {
    this->AddOrReplaceTexture(false);
}

void MainWindow::on_actionRemove_texture_triggered() {
    if (mTexturesDB && this->GetSelectedTextureIdx() >= 0) {
        const size_t idx = scast<size_t>(this->GetSelectedTextureIdx());

        MetroTextureInfoCommon info;
        mTexturesDB->FillCommonInfoByIdx(idx, info);

        QString fullQuestion = tr("Are you sure you wan to remove\n") + QString::fromStdString(info.name) + " ?";
        const QMessageBox::StandardButton answer = QMessageBox::question(this, this->windowTitle(), fullQuestion);

        if (answer == QMessageBox::Yes) {
            mTexturesDB->RemoveTextureByIdx(idx);

            const int selectedRow = ui->lstTextures->currentRow();
            ui->lstTextures->removeItemWidget(ui->lstTextures->item(selectedRow));

            if (selectedRow < ui->lstTextures->count()) {
                ui->lstTextures->setCurrentRow(selectedRow);
            }
        }
    }
}

void MainWindow::on_actionShow_transparency_triggered() {
    mImagePanel->ShowTransparency(ui->actionShow_transparency->isChecked());
}

void MainWindow::on_actionCalculate_texture_average_colour_triggered() {
    const void* imageData = mImagePanel->GetImageData();
    if (mTexturesDB && this->GetSelectedTextureIdx() >= 0 && imageData) {
        const size_t width = mImagePanel->GetImageWidth();
        const size_t height = mImagePanel->GetImageHeight();

        const uint8_t* rgbValues = rcast<const uint8_t*>(imageData);
        double avgR = 0, avgG = 0, avgB = 0;
        for (size_t y = 0; y < height; ++y) {
            double avgLineR = 0, avgLineG = 0, avgLineB = 0;
            for (size_t x = 0; x < width; ++x, rgbValues += 4) {
                avgLineR += scast<double>(rgbValues[0]);
                avgLineG += scast<double>(rgbValues[1]);
                avgLineB += scast<double>(rgbValues[2]);
            }

            avgR += avgLineR / scast<double>(width);
            avgG += avgLineG / scast<double>(width);
            avgB += avgLineB / scast<double>(width);
        }

        avgR /= scast<double>(height);
        avgG /= scast<double>(height);
        avgB /= scast<double>(height);

        const uint8_t R = scast<uint8_t>(avgR);
        const uint8_t G = scast<uint8_t>(avgG);
        const uint8_t B = scast<uint8_t>(avgB);

        const uint32_t avgColour = (R << 24) | (G << 16) | (B << 8);

        MetroTextureInfoCommon info;
        mTexturesDB->FillCommonInfoByIdx(scast<size_t>(this->GetSelectedTextureIdx()), info);
        info.avg_color = avgColour;
        mTexturesDB->SetCommonInfoByIdx(scast<size_t>(this->GetSelectedTextureIdx()), info);

        mPropertyBrowser->setActiveObject(nullptr);
        mTextureInfoProps = MakeStrongPtr<MetroTextureInfoData>();
        TextureCommonInfoToTextureInfoProps2033(info, mTextureInfoProps.get());
        mPropertyBrowser->setActiveObject(mTextureInfoProps.get());
    }
}

void MainWindow::onPropertyBrowserObjectPropertyChanged() {
    if (mTexturesDB && this->GetSelectedTextureIdx() >= 0) {
        MetroTextureInfoCommon info;
        TextureInfoProps2033ToTextureCommonInfo(mTextureInfoProps.get(), info);

        mTexturesDB->SetCommonInfoByIdx(scast<size_t>(this->GetSelectedTextureIdx()), info);
    }
}

void MainWindow::onSearchTimerTick() {
    mSearchTimer->stop();

    QString text = ui->txtSearch->text().trimmed();
    this->FillTexturesList(text);
}

void MainWindow::on_txtSearch_textEdited(const QString&) {
    if (mTexturesDB) {
        mSearchTimer->stop();
        mSearchTimer->start();
    }
}

int MainWindow::GetSelectedTextureIdx() const {
    if (mTexturesDB && ui->lstTextures->currentItem()) {
        const int idx = ui->lstTextures->currentItem()->data(Qt::UserRole).toInt();
        return idx;
    } else {
        return -1;
    }
}
