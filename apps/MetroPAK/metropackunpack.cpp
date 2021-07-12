#include "metropackunpack.h"

#include "metro/MetroContext.h"

#include <fstream>

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

struct FileEntry {
    fs::path    path;
    uint32_t    size;
    uint32_t    crc32;
    uint32_t    offset;
};

// returns CRC32, or null if failed
uint32_t AppendFileContent(const fs::path& filePath, std::ofstream& dst) {
    std::ifstream src(filePath, std::ofstream::in | std::ofstream::binary);
    if (src.is_open()) {
        Crc32Stream crcStream;

        const size_t kBufferSize = 128 * 1024;
        MyArray<char> buffer(kBufferSize);

        while (!src.eof()) {
            src.read(buffer.data(), kBufferSize);
            const size_t bytesRead = src.gcount();
            dst.write(buffer.data(), bytesRead);

            crcStream.Update(buffer.data(), bytesRead);
        }

        return crcStream.Finalize();
    } else {
        return 0u;
    }
}

void MetroPackUnpack::PackArchive2033(const fs::path& contentFolderPath, const fs::path& archivePath, std::function<bool(float)> progress) {
    MyArray<FileEntry> filesList;
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(contentFolderPath)) {
        const fs::path& path = entry.path();
        if (OSPathIsFile(path)) {
            filesList.push_back({
                path,
                scast<uint32_t>(OSGetFileSize(path)),
                0u,
                0u
            });
        }
    }

    std::ofstream archiveFile(archivePath, std::ios_base::out | std::ios_base::binary);
    if (archiveFile.is_open()) {
        auto writeU32 = [&archiveFile](const uint32_t v) {
            archiveFile.write(rcast<const char*>(&v), sizeof(v));
        };

        fs::path contentBasePath = contentFolderPath.parent_path();

        uint32_t blobChunkSize = 0;
        uint32_t tocChunkSize = 0;
        uint32_t fileOffset = 0;

        // blob chunk
        writeU32(0u);               // id
        writeU32(blobChunkSize);    // size (placeholder for now)

        fileOffset += 8;

        const size_t numFilesTotal = filesList.size();
        size_t filesWritten = 0;

        for (FileEntry& entry : filesList) {
            entry.crc32 = AppendFileContent(entry.path, archiveFile);
            entry.offset = fileOffset;

            fileOffset += entry.size;

            filesWritten++;
            const bool okToProceed = progress(scast<float>(filesWritten) / scast<float>(numFilesTotal));
            if (!okToProceed) {
                break;
            }
        }
        blobChunkSize = fileOffset - 8;

        // TOC chunk
        writeU32(1u);               // id
        const size_t tocSizeOffset = archiveFile.tellp();
        writeU32(tocChunkSize);     // size (placeholder for now)

        for (const FileEntry& entry : filesList) {
            writeU32(entry.crc32);
            writeU32(entry.offset);
            writeU32(entry.size);
            writeU32(entry.size);

            CharString fileName = fs::relative(entry.path, contentBasePath).string();
            writeU32(scast<uint32_t>(fileName.length() + 1));

            const char xorValue = scast<char>(entry.crc32 & 0xFF);
            std::transform(fileName.begin(), fileName.end(), fileName.begin(), [xorValue](char c)->char { return c ^ xorValue; });
            archiveFile.write(fileName.data(), fileName.length() + 1);
        }
        tocChunkSize = scast<uint32_t>(scast<size_t>(archiveFile.tellp()) - tocSizeOffset - 4);

        // chunk 2
        writeU32(2u);   // id
        writeU32(4u);   // size
        writeU32(0u);   // content

        // chunk 3
        writeU32(3u);   // id
        writeU32(16u);  // size
        archiveFile.write("\x29\x2C\x55\xF8\xAC\xA8\x96\x41\xA9\xA4\xB1\xB4\x7D\xB0\x65\x4F", 16);

        archiveFile.flush();

        // post-fix blob chunk size
        archiveFile.seekp(4);
        writeU32(blobChunkSize);

        // post-fix TOC chunk size
        archiveFile.seekp(tocSizeOffset);
        writeU32(tocChunkSize);

        archiveFile.flush();
        archiveFile.close();
    }
}
