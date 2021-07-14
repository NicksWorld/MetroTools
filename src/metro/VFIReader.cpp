#include "VFIReader.h"
#include "MetroCompression.h"

#include <fstream>

static const size_t kVFIChunk_Header        = kInvalidValue32;
static const size_t kVFIPackageChunk_Header = 0;
static const size_t kVFIPackageChunk_Files  = 1;

VFIReader::VFIReader()
    : mVersion(0)
    , mRootFolderIdx(0)
{
}
VFIReader::~VFIReader() {
}

bool VFIReader::LoadFromFile(const fs::path& filePath) {
    bool result = false;

    this->Close();

    LogPrint(LogLevel::Info, "Loading vfi file...");

    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        BytesArray fileData;

        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        MemStream stream(fileData.data(), fileData.size());

        while (stream.Remains()) {
            const size_t chunkId = stream.ReadTyped<uint32_t>();
            const size_t chunkSize = stream.ReadTyped<uint32_t>();
            MemStream chunkStream = stream.Substream(chunkSize);

            switch (chunkId) {
                case kVFIChunk_Header: {
                    mVersion = chunkStream.ReadTyped<uint32_t>();
                    chunkStream.ReadStruct(mGUID);

                    LogPrint(LogLevel::Info, CharString("VFI version = ") + std::to_string(mVersion));
                    LogPrintF(LogLevel::Info, "VFI guid = %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
                        mGUID.a, mGUID.b, mGUID.c, mGUID.d, mGUID.e[0], mGUID.e[1], mGUID.e[2], mGUID.e[3], mGUID.e[4], mGUID.e[5]);
                } break;

                //#NOTE_SK: any non-header chunk id is basically a package id
                default: {
                    this->ReadPackage(chunkStream);
                } break;
            }

            stream.SkipBytes(chunkSize);
        }

        result = !mFiles.empty();

        if (result) {
            mBasePath = filePath.parent_path();
            mFileName = filePath.filename().string();
            mAbsolutePath = fs::absolute(filePath);

            this->BuildFileTree();
        }
    }

    return result;
}

void VFIReader::Close() {
    mPackages.clear();
    mFiles.clear();
}

size_t VFIReader::GetVersion() const {
    return mVersion;
}

const MetroGuid& VFIReader::GetGUID() const {
    return mGUID;
}

const CharString& VFIReader::GetSelfName() const {
    return mFileName;
}

const fs::path& VFIReader::GetAbsolutePath() const {
    return mAbsolutePath;
}

size_t VFIReader::GetRootFolderIdx() const {
    return mRootFolderIdx;
}

bool VFIReader::IsFolder(const size_t idx) const {
    return mFiles[idx].IsFolder();
}

const CharString& VFIReader::GetFileName(const size_t idx) const {
    return mFiles[idx].name.str;
}

size_t VFIReader::GetSizeUncompressed(const size_t idx) const {
    return mFiles[idx].sizeUncompressed;
}

size_t VFIReader::GetSizeCompressed(const size_t idx) const {
    return mFiles[idx].sizeCompressed;
}

const MyArray<size_t>& VFIReader::GetChildren(const size_t idx) const {
    return mFiles[idx].children;
}

MemStream VFIReader::ExtractFile(const size_t fileIdx, const size_t subOffset, const size_t subLength) const {
    MemStream result;

    const File& mf = mFiles[fileIdx];
    const Package& pak = mPackages[mf.packIdx];

    fs::path pakPath = mBasePath / pak.name;
    std::ifstream file(pakPath, std::ifstream::binary);
    if (file.good()) {
        file.seekg(mf.offset);

        const size_t streamOffset = (subOffset == kInvalidValue) ? 0 : std::min<size_t>(subOffset, mf.sizeUncompressed);
        const size_t streamLength = (subLength == kInvalidValue) ? (mf.sizeUncompressed - streamOffset) : (std::min<size_t>(subLength, mf.sizeUncompressed - streamOffset));

        uint8_t* fileContent = rcast<uint8_t*>(malloc(mf.sizeCompressed));
        file.read(rcast<char*>(fileContent), mf.sizeCompressed);

        if (mf.sizeCompressed == mf.sizeUncompressed) {
            result = MemStream(fileContent, mf.sizeUncompressed, true);
        } else {
            uint8_t* uncompressedContent = rcast<uint8_t*>(malloc(mf.sizeUncompressed));
            const size_t decompressResult = MetroCompression::DecompressStreamLegacy(fileContent, mf.sizeCompressed, uncompressedContent, mf.sizeUncompressed);

            if (decompressResult == mf.sizeUncompressed) {
                result = MemStream(uncompressedContent, mf.sizeUncompressed, true);
            }

            free(fileContent);
        }

        if (result.Good()) {
            result.SetWindow(streamOffset, streamLength);
        }
    }

    return result;
}

void VFIReader::ReadPackage(MemStream& stream) {
    const size_t thisPackIdx = mPackages.size();

    while (stream.Remains()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        MemStream chunkStream = stream.Substream(chunkSize);

        switch (chunkId) {
            case kVFIPackageChunk_Header: {
                mPackages.resize(mPackages.size() + 1);
                Package& pak = mPackages.back();
                pak.name = chunkStream.ReadStringZ();
                pak.size = chunkStream.ReadTyped<uint32_t>();
            } break;

            case kVFIPackageChunk_Files: {
                while (chunkStream.Remains()) {
                    uint32_t hdr = chunkStream.ReadTyped<uint32_t>();

                    File file;
                    file.packIdx = thisPackIdx;
                    file.offset = chunkStream.ReadTyped<uint32_t>();
                    file.sizeUncompressed = chunkStream.ReadTyped<uint32_t>();
                    file.sizeCompressed = chunkStream.ReadTyped<uint32_t>();

                    const size_t nameLength = chunkStream.ReadTyped<uint32_t>();
                    file.fullPath.reserve(nameLength);

                    const char xorMask = scast<char>(hdr & 0xFF);
                    for (size_t i = 0; i < nameLength - 1; ++i) {
                        const char ch = chunkStream.ReadTyped<char>();
                        file.fullPath.push_back(ch ^ xorMask);
                    }
                    chunkStream.SkipBytes(1); // trailing null

                    CharString::size_type lastSlashPos = file.fullPath.find_last_of('\\');
                    if (lastSlashPos != CharString::npos) {
                        file.name = file.fullPath.substr(lastSlashPos + 1);
                    }

                    mFiles.emplace_back(file);
                }
            } break;
        }

        stream.SkipBytes(chunkSize);
    }
}

void VFIReader::BuildFileTree() {
    const size_t end = mFiles.size();

    mRootFolderIdx = end;
    File rootFolder = {};
    rootFolder.name = CharString("content");
    mFiles.emplace_back(rootFolder);

    for (size_t i = 0; i < end; ++i) {
        const File& file = mFiles[i];
        CharString fullPath = file.fullPath;

        size_t parentFolderIdx = mRootFolderIdx;
        size_t pathLeft = fullPath.find_first_of('\\') + 1, pathRight; //#NOTE_SK: we do this once before loop to skip "content"

        for (pathRight = fullPath.find_first_of('\\', pathLeft); pathRight != CharString::npos; pathLeft = pathRight + 1, pathRight = fullPath.find_first_of('\\', pathLeft)) {
            CharString folderName = fullPath.substr(pathLeft, pathRight - pathLeft);
            parentFolderIdx = this->GetOrAddFolder(folderName, parentFolderIdx);
        }

        File& parentFolder = mFiles[parentFolderIdx];
        parentFolder.children.push_back(i);
    }
}

size_t VFIReader::GetOrAddFolder(const HashString& folderName, const size_t parentFolder) {
    size_t result = kInvalidValue;

    const File& parent = mFiles[parentFolder];
    for (const size_t idx : parent.children) {
        File& child = mFiles[idx];
        if (child.name == folderName) {
            result = idx;
            break;
        }
    }

    if (kInvalidValue == result) {
        const size_t newIdx = mFiles.size();
        File newFolder = {};
        newFolder.name = folderName;
        mFiles.emplace_back(newFolder);

        mFiles[parentFolder].children.push_back(newIdx);

        result = newIdx;
    }

    return result;
}
