#include "metropackunpack.h"

#include "metro/MetroContext.h"
#include "metro/MetroCompression.h"
#include "metro/VFXReader.h"

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
    if (WStrStartsWith(extension, L".upk")) {
        ok = mfs.InitFromSingleUPK(archivePath);
    } else if (WStrStartsWith(extension, L".vfi")) {
        ok = mfs.InitFromSingleVFI(archivePath);
    } else {
        ok = mfs.InitFromSingleVFX(archivePath);
    }

    if (ok) {
        const bool isPatch = WStrStartsWith(archivePath.stem().wstring(), L"patch");
        size_t numPatchFiles = 0;
        if (isPatch) {
            const size_t numVfx = mfs.GetNumVFX();
            for (size_t i = 0; i < numVfx; ++i) {
                const VFXReader* vfx = mfs.GetVFX(i);
                const MyArray<size_t>& allFolders = vfx->GetAllFolders();
                for (const size_t folderIdx : allFolders) {
                    const MetroFile& folder = vfx->GetFile(folderIdx);
                    if (!folder.IsPatchFolder() && folder.name != "content") {
                        numPatchFiles += folder.numFiles;
                    }
                }
            }
        }

        const size_t numFilesTotal = mfs.CountFilesInFolder(mfs.GetRootFolder(), true) + numPatchFiles;

        ExtractContext ctx = {
            &mfs,
            mfs.GetRootFolder(),
            numFilesTotal,
            0,
            progress
        };

        ExtractFolderComplete(ctx, outputFolderPath);

        if (isPatch) {
            const size_t numVfx = mfs.GetNumVFX();
            for (size_t i = 0; i < numVfx; ++i) {
                const VFXReader* vfx = mfs.GetVFX(i);
                const MyArray<size_t>& allFolders = vfx->GetAllFolders();
                for (const size_t folderIdx : allFolders) {
                    const MetroFile& folder = vfx->GetFile(folderIdx);
                    if (!folder.IsPatchFolder() && folder.name != "content") {
                        fs::path fullPath = outputFolderPath / fs::path(folder.name);
                        fs::create_directories(fullPath);

                        for (auto f : folder) {
                            const MetroFile& file = vfx->GetFile(f);
                            if (file.IsFile()) {
                                fs::path filePath = fullPath / file.name;
                                MemStream stream = vfx->ExtractFile(file.idx);
                                OSWriteFile(filePath, stream.Data(), stream.Length());

                                ctx.extractedFiles++;
                                const bool okToProceed = ctx.progress(scast<float>(ctx.extractedFiles) / scast<float>(ctx.numFilesTotal));
                                if (!okToProceed) {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

struct FileEntry {
    fs::path    path;
    uint32_t    size;
    uint32_t    sizeCompressed;
    uint32_t    crc32;
    uint32_t    offset;
};

// returns CRC32, or null if failed
static uint32_t AppendFileContent(const fs::path& filePath, std::ofstream& dst) {
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

// returns CRC32 and compressed size, or (null, null) if failed
static std::pair<uint32_t, uint32_t> CompressAndAppendFileContent(const fs::path& filePath, std::ofstream& dst) {
    std::pair<uint32_t, uint32_t> result = { 0u, 0u };

    MemStream fileStream = OSReadFile(filePath);
    if (fileStream.Good()) {
        BytesArray compressed;
        MetroCompression::CompressStreamLegacy(fileStream.Data(), fileStream.Length(), compressed);

        result.first = Hash_CalculateCRC32(compressed.data(), compressed.size());
        result.second = scast<uint32_t>(compressed.size());

        dst.write(rcast<const char*>(compressed.data()), compressed.size());
    }

    return result;
}

static bool ShouldCompressFile(const bool useCompression, const CharString& fileName) {
    if (!useCompression) {
        return false;
    } else {
        CharString extension;
        CharString::size_type lastDotPos = fileName.find_last_of('\\');
        if (lastDotPos != CharString::npos) {
            extension = fileName.substr(lastDotPos + 1);
        }

        // these type of files are never compressed in 2033
        if (extension == "ogv" ||
            extension == "ogg" ||
            extension == "pe") {
            return false;
        } else {
            return true;
        }
    }
}

void MetroPackUnpack::PackArchive2033(const fs::path& contentFolderPath, const fs::path& archivePath, const bool useCompression, std::function<bool(float)> progress) {
    MyArray<FileEntry> filesList;
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(contentFolderPath)) {
        const fs::path& path = entry.path();
        if (OSPathIsFile(path)) {
            filesList.push_back({
                path,
                scast<uint32_t>(OSGetFileSize(path)),
                0u,
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
            if (!ShouldCompressFile(useCompression, entry.path.filename().string())) {
                entry.crc32 = AppendFileContent(entry.path, archiveFile);
                entry.sizeCompressed = entry.size;
            } else {
                auto [crc32, compressedSize] = CompressAndAppendFileContent(entry.path, archiveFile);
                entry.crc32 = crc32;
                entry.sizeCompressed = compressedSize;
            }

            entry.offset = fileOffset;

            fileOffset += entry.sizeCompressed;

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
            writeU32(entry.sizeCompressed);

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
