#include "VFXReader.h"
#include "MetroCompression.h"

#include <fstream>


bool VFXReader::IsGUIDLastLight(const MetroGuid& guid) {
    return guid == VFXReader::kGUIDLastLightSteam ||
           guid == VFXReader::kGUIDLastLightSteamLinux ||
           guid == VFXReader::kGUIDLastLightSteamMacOS ||
           guid == VFXReader::kGUIDLastLightXBox360 ||
           guid == VFXReader::kGUIDLastLightXBox360_Build_Oct18_2012 ||
           guid == VFXReader::kGUIDLastLightXBox360_Build_Dec03_2012;
    return true;
}

VFXReader::VFXReader()
    : mVersion(kVFXVersionExodus)
    , mCompressionType(MetroCompression::Type_Unknown)
    , mIsLastLight(false) {
}

VFXReader::~VFXReader() {
}

bool VFXReader::LoadFromFile(const fs::path& filePath) {
    bool result = false;

    this->Close();

    LogPrint(LogLevel::Info, "loading vfx file...");

    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        BytesArray fileData;

        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        MemStream stream(fileData.data(), fileData.size());

        mVersion = stream.ReadTyped<uint32_t>();
        mCompressionType = stream.ReadTyped<uint32_t>();

        LogPrint(LogLevel::Info, "vfx version = " + std::to_string(mVersion) + ", compression = " + std::to_string(mCompressionType));

        if ((mVersion > kVFXVersionUnknown && mVersion < kVFXVersionMax) && mCompressionType == MetroCompression::Type_LZ4) {
            if (mVersion >= kVFXVersionExodus) {
                mContentVersion = stream.ReadStringZ();
            }
            stream.ReadStruct(mGUID); // guid, seems to be static across the game
            const size_t numVFS = stream.ReadTyped<uint32_t>();
            const size_t numFiles = stream.ReadTyped<uint32_t>();
            const size_t numDuplicates = stream.ReadTyped<uint32_t>();

            LogPrint(LogLevel::Info, "vfx content version = " + mContentVersion);
            LogPrintF(LogLevel::Info, "vfx guid = %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
                mGUID.a, mGUID.b, mGUID.c, mGUID.d, mGUID.e[0], mGUID.e[1], mGUID.e[2], mGUID.e[3], mGUID.e[4], mGUID.e[5]);

            LogPrintF(LogLevel::Info, "packages = %lld, files = %lld, duplicates = %lld", numVFS, numFiles, numDuplicates);

            mIsLastLight = IsGUIDLastLight(mGUID);
            if (mIsLastLight) {
                LogPrint(LogLevel::Info, "based on a GUID this is a Last Light vfx...");
            }

            mPaks.resize(numVFS);
            for (Package& pak : mPaks) {
                pak.name = stream.ReadStringZ();

                if (mIsLastLight) {
                    const uint32_t vfsSize = stream.ReadTyped<uint32_t>();
                    LogPrintF(LogLevel::Info, "blob %s, size %lld", pak.name.c_str(), vfsSize);
                } else {
                    const uint32_t numStrings = stream.ReadTyped<uint32_t>();
                    pak.levels.resize(numStrings);
                    for (auto& s : pak.levels) {
                        s = stream.ReadStringZ();
                    }

                    pak.chunk = stream.ReadTyped<uint32_t>();
                }
            }

            mFiles.resize(numFiles);
            size_t fileIdx = 0;
            for (MetroFile& mf : mFiles) {
                mf.idx = fileIdx;

                this->ReadFileDescription(mf, stream, false, mIsLastLight);

                if (!mf.IsFile()) {
                    mFolders.push_back(fileIdx);
                }

                ++fileIdx;
            }

            const bool weirdDuplicatesSerialization = (mIsLastLight && (mGUID == VFXReader::kGUIDLastLightXBox360_Build_Oct18_2012));

            mDuplicates.resize(numDuplicates);
            size_t duplicateIdx = 0;
            for (MetroFile& mf : mDuplicates) {
                if (weirdDuplicatesSerialization) {
                    this->ReadFileDescription(mf, stream, false, mIsLastLight);
                } else {
                    this->ReadFileDescription(mf, stream, true, mIsLastLight);

                    MetroFile& baseMf = mFiles[mf.baseIdx];
                    mf.duplicates = baseMf.duplicates;
                    baseMf.duplicates = duplicateIdx;

                    mf.name = baseMf.name;
                }

                ++duplicateIdx;
            }

            mBasePath = filePath.parent_path();
            mFileName = filePath.filename().string();
            mAbsolutePath = fs::absolute(filePath);
            result = true;

            LogPrint(LogLevel::Info, "vfx loaded successfully");
        } else {
            LogPrint(LogLevel::Error, "unknown version or compression");
        }
    } else {
        LogPrint(LogLevel::Error, "failed to open file");
    }

    return result;
}

void WriteStringZ(std::ofstream& stream, const CharString& str) {
    stream.write(str.data(), str.length() + 1);
}

void WriteStringXored(std::ofstream& stream, const CharString& str) {
    static const char sEmpty[3] = { 1, 0, 0 };
    if (str.empty()) {
        stream.write(sEmpty, sizeof(sEmpty));
    } else {
        BytesArray temp(str.length() + 1);
        memcpy(temp.data(), str.data(), temp.size());

        const uint8_t xorMask = scast<uint8_t>(rand() % 235) + 15;
        for (size_t i = 0, end = temp.size() - 1; i < end; ++i) {
            temp[i] ^= xorMask;
        }

        const uint16_t header = (scast<uint16_t>(xorMask) << 8) | scast<uint16_t>(temp.size() & 0xFF);
        stream.write(rcast<const char*>(&header), sizeof(header));
        stream.write(rcast<const char*>(temp.data()), temp.size());
    }
}

void WriteU16(std::ofstream& stream, const uint16_t value) {
    stream.write(rcast<const char*>(&value), sizeof(value));
}

void WriteU32(std::ofstream& stream, const uint32_t value) {
    stream.write(rcast<const char*>(&value), sizeof(value));
}

bool VFXReader::SaveToFile(const fs::path& filePath) const {
    bool result = false;

    std::ofstream vfxFile(filePath, std::ofstream::binary);
    if (vfxFile.good()) {
        // write header
        WriteU32(vfxFile, scast<uint32_t>(mVersion));
        WriteU32(vfxFile, scast<uint32_t>(mCompressionType));
        if (mVersion >= kVFXVersionExodus) {
            WriteStringZ(vfxFile, mContentVersion);
        }
        vfxFile.write(rcast<const char*>(&mGUID), sizeof(mGUID));
        WriteU32(vfxFile, scast<uint32_t>(mPaks.size()));
        WriteU32(vfxFile, scast<uint32_t>(mFiles.size()));
        WriteU32(vfxFile, 0); // duplicates

        // write package
        for (auto& pak : mPaks) {
            WriteStringZ(vfxFile, pak.name);
            WriteU32(vfxFile, scast<uint32_t>(pak.levels.size()));
            for (auto& s : pak.levels) {
                WriteStringZ(vfxFile, s);
            }
            WriteU32(vfxFile, scast<uint32_t>(pak.chunk));
        }

        for (auto& mf : mFiles) {
            WriteU16(vfxFile, scast<uint16_t>(mf.flags));
            if (mf.IsFile()) {
                WriteU16(vfxFile, scast<uint16_t>(mf.pakIdx));
                WriteU32(vfxFile, scast<uint32_t>(mf.offset));
                WriteU32(vfxFile, scast<uint32_t>(mf.sizeUncompressed));
                WriteU32(vfxFile, scast<uint32_t>(mf.sizeCompressed));
            } else {
                WriteU16(vfxFile, scast<uint16_t>(mf.numFiles));
                WriteU32(vfxFile, scast<uint32_t>(mf.firstFile));
            }
            WriteStringXored(vfxFile, mf.name);
        }
        WriteU32(vfxFile, 0);
        WriteU32(vfxFile, 0);

        vfxFile.flush();
        vfxFile.close();

        result = true;
    }

    return result;
}

void VFXReader::Close() {
    mPaks.resize(0);
    mFiles.resize(0);
    mFolders.resize(0);
    mDuplicates.resize(0);
}

const CharString& VFXReader::GetContentVersion() const {
    return mContentVersion;
}

size_t VFXReader::GetVersion() const {
    return mVersion;
}

const MetroGuid& VFXReader::GetGUID() const {
    return mGUID;
}

MemStream VFXReader::ExtractFile(const size_t fileIdx, const size_t subOffset, const size_t subLength) const {
    MemStream result;

    const MetroFile& mf = mFiles[fileIdx];
    const Package& pak = mPaks[mf.pakIdx];

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
            const size_t decompressResult = mIsLastLight ?
                MetroCompression::DecompressStreamLegacy(fileContent, mf.sizeCompressed, uncompressedContent, mf.sizeUncompressed) :
                MetroCompression::DecompressStream(fileContent, mf.sizeCompressed, uncompressedContent, mf.sizeUncompressed);

            if (decompressResult == mf.sizeUncompressed) {
                result = MemStream(uncompressedContent, mf.sizeUncompressed, true);
            }

            free(fileContent);
        }

        if (result.Good()) {
            result.SetWindow(streamOffset, streamLength);
        }
    }

    return std::move(result);
}

bool VFXReader::Good() const {
    return !mFiles.empty();
}

const CharString& VFXReader::GetSelfName() const {
    return mFileName;
}

const fs::path& VFXReader::GetAbsolutePath() const {
    return mAbsolutePath;
}

const MyArray<Package>& VFXReader::GetAllPacks() const {
    return mPaks;
}

const MyArray<MetroFile>& VFXReader::GetAllFiles() const {
    return mFiles;
}

const MyArray<size_t>& VFXReader::GetAllFolders() const {
    return mFolders;
}

const MetroFile& VFXReader::GetRootFolder() const {
    return mFiles.front();
}

const MetroFile& VFXReader::GetFile(const size_t idx) const {
    return mFiles[idx];
}


// modification
void VFXReader::AddPackage(const Package& pak) {
    mPaks.push_back(pak);
}

void VFXReader::ReplaceFileInfo(const size_t idx, const MetroFile& newFile) {
    mFiles[idx] = newFile;
}

void VFXReader::AppendFolder(const MetroFile& folder) {
    mFiles.push_back(folder);

    const size_t firstIdx = mFiles.size();
    MetroFile& mf = mFiles.back();
    mf.firstFile = firstIdx;
    mf.numFiles = 0;
}

static CharString ReadEncryptedFileName(MemStream& stream) {
    CharString result;

    const uint16_t stringHeader = stream.ReadTyped<uint16_t>();
    const size_t stringLen = (stringHeader & 0xFF);
    const char xorMask = scast<char>((stringHeader >> 8) & 0xFF);

    result.reserve(stringLen);
    for (size_t i = 1; i < stringLen; ++i) {
        const char ch = stream.ReadTyped<char>();
        result.push_back(ch ^ xorMask);
    }

    stream.ReadTyped<char>(); // terminating null

    return result;
};

void VFXReader::ReadFileDescription(MetroFile& mf, MemStream& stream, const bool isDuplicate, const bool isLastLight) {
    mf.flags = stream.ReadTyped<uint16_t>();

    if (isLastLight) {
        //#NOTE_SK: in Last Light folder flag is 16 ... make it 8 so it's compatible with later VFX versions
        mf.flags >>= 1;
    }

    if (mf.IsFile()) {
        mf.pakIdx = stream.ReadTyped<uint16_t>();
        mf.offset = stream.ReadTyped<uint32_t>();
        mf.sizeUncompressed = stream.ReadTyped<uint32_t>();
        mf.sizeCompressed = stream.ReadTyped<uint32_t>();

        if (isDuplicate) {
            mf.baseIdx = stream.ReadTyped<uint32_t>();
        }

        mf.duplicates = kInvalidValue;
    } else {
        mf.numFiles = stream.ReadTyped<uint16_t>();
        mf.firstFile = stream.ReadTyped<uint32_t>();
    }

    if (!isDuplicate) {
        mf.name = ReadEncryptedFileName(stream);
    }
}
