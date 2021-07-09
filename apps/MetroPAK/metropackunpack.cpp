#include "metropackunpack.h"

#include "metro/MetroContext.h"

struct ExtractContext {
    MetroFileSystem*            mfs;
    MetroFSPath                 file;
    size_t                      numFilesTotal;
    size_t                      extractedFiles;
    std::function<bool(float)>  progress;
};

static bool ExtractFolderComplete(ExtractContext& ctx, const fs::path& outPath) {
    MetroFileSystem& mfs = *ctx.mfs;
    const CharString& folderName = mfs.GetName(ctx.file);

    fs::path curPath = outPath / folderName;
    fs::create_directories(curPath);

    ExtractContext tmpCtx = ctx;
    for (MyHandle child = mfs.GetFirstChild(ctx.file.fileHandle); child != kInvalidHandle; child = mfs.GetNextChild(child)) {
        tmpCtx.file = MetroFSPath(child);

        if (mfs.IsFolder(tmpCtx.file)) {
            if (!ExtractFolderComplete(tmpCtx, curPath)) {
                return false;
            }
        } else {
            const CharString& childName = mfs.GetName(tmpCtx.file);
            fs::path filePath = curPath / childName;
            MemStream stream = mfs.OpenFileStream(tmpCtx.file);
            OSWriteFile(filePath, stream.Data(), stream.Length());

            tmpCtx.extractedFiles++;
            const bool okToProceed = ctx.progress(scast<float>(tmpCtx.extractedFiles) / scast<float>(ctx.numFilesTotal));
            if (!okToProceed) {
                return false;
            }
        }
    }

    ctx.extractedFiles = tmpCtx.extractedFiles;

    return true;
}

void MetroPackUnpack::UnpackArchive(const fs::path& archivePath, const fs::path& outputFolderPath, std::function<bool(float)> progress) {
    MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    WideString extension = archivePath.extension().wstring();

    bool ok = false;
    if (WStrStartsWith(extension, L".vfi")) {
        ok = mfs.InitFromSingleVFI(archivePath);
    } else {
        ok = mfs.InitFromSingleVFX(archivePath);
    }

    if (ok) {
        const size_t numFilesTotal = mfs.CountFilesInFolder(mfs.GetRootFolder(), true);

        ExtractContext ctx = {
            &mfs,
            mfs.GetRootFolder(),
            numFilesTotal,
            0,
            progress
        };

        ExtractFolderComplete(ctx, outputFolderPath);
    }
}

void MetroPackUnpack::PackArchive2033(const fs::path& contentFolderPath, const fs::path& archivePath, std::function<bool(float)> progress) {

}
