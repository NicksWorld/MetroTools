#include "MetroVFS.h"
#include "metro/MetroCompression.h"
#include "log.h"

#include <fstream>


// Where in the fuck is version 2 ????
static const size_t kVFXVersionUnknown          = 0;
static const size_t kVFXVersionLL_Redux_Arktika = 1;
static const size_t kVFXVersionExodus           = 3;

static bool IsGUIDLastLight(const MetroGuid&) {
    return false;
}

static const CharString sExodusPatchArchives[] = {
    "patch.vfx0",
    "patch_00.vfx",
    "patch_01_shared.vfx",
    "patch_01.vfx",
    "patch_02.vfx",
    "patch_03.vfx",
    "patch_04.vfx",
};


MetroVFS::MetroVFS() {}
MetroVFS::~MetroVFS() {}


bool MetroVFS::LoadGameFolder(const fs::path& gameFolder) {
    bool result = this->LoadContent(gameFolder / "content.vfx");
    if (result) {
        if (mVersion == kVFXVersionExodus) {
            size_t layer = 0;
            for (const CharString& patchName : sExodusPatchArchives) {
                this->LoadPatch(gameFolder / patchName, ++layer);
            }
        } else {
            for (size_t layer = 1; layer < kMaxLayers; ++layer) {
                CharString patchName = "patch.vfx" + std::to_string(layer - 1);
                this->LoadPatch(gameFolder / patchName, layer);
            }
        }
    }

    return result;
}

bool MetroVFS::LoadSingleRegistry(const fs::path& filePath) {
    const bool isContent = filePath.filename() == "content.vfx";
    if (isContent) {
        return this->LoadContent(filePath);
    } else {
        return this->LoadPatch(filePath, kBaseLayer);
    }
}

MetroVFS::VFSFile* MetroVFS::FindByName(const StringView& path) {
    if (mFiles[kBaseLayer].empty()) {
        return nullptr;
    } else {
        return this->Find(kBaseLayer, &mFiles[kBaseLayer].front(), path, 0, nullptr);
    }
}


bool MetroVFS::LoadRegistryHeader(MemStream& stream) {
    bool result = false;

    mVersion = stream.ReadTyped<uint32_t>();
    mCompressionType = stream.ReadTyped<uint32_t>();

    LogPrint(LogLevel::Info, "vfx version = " + std::to_string(mVersion) + ", compression = " + std::to_string(mCompressionType));

    if ((mVersion >= kVFXVersionLL_Redux_Arktika && mVersion <= kVFXVersionExodus) && mCompressionType == MetroCompression::Type_LZ4) {
        if (mVersion >= kVFXVersionExodus) {
            mContentVersion = stream.ReadStringZ();
        }
        stream.ReadStruct(mGuid); // guid, seems to be constant across the game
        mNumPackages = stream.ReadTyped<uint32_t>();
        mNumFiles = stream.ReadTyped<uint32_t>();
        mNumDuplicates = stream.ReadTyped<uint32_t>();

        LogPrint(LogLevel::Info, "vfx content version = " + mContentVersion);
        LogPrintF(LogLevel::Info, "vfx guid = %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            mGuid.a, mGuid.b, mGuid.c, mGuid.d, mGuid.e[0], mGuid.e[1], mGuid.e[2], mGuid.e[3], mGuid.e[4], mGuid.e[5]);

        LogPrintF(LogLevel::Info, "packages = %lld, files = %lld, duplicates = %lld", mNumPackages, mNumFiles, mNumDuplicates);

        mIsLastLight = IsGUIDLastLight(mGuid);
        if (mIsLastLight) {
            LogPrint(LogLevel::Info, "based on a GUID this is a Last Light vfx...");
        }

        mPackages.resize(mNumPackages);
        for (VFSPackage& pak : mPackages) {
            pak.name = stream.ReadStringZ();

            LogPrintF(LogLevel::Info, "package %s", pak.name.c_str());

            if (!mIsLastLight) {
                const uint32_t numLevels = stream.ReadU32();
                pak.levels.resize(numLevels);
                for (auto& s : pak.levels) {
                    s = stream.ReadStringZ();

                    LogPrintF(LogLevel::Info, "package contains level %s", s.c_str());
                }
            }

            pak.size = stream.ReadU32();
            LogPrintF(LogLevel::Info, "package size = %lld", pak.size);
        }

        result = true;
    }

    return result;
}

void MetroVFS::ReadFileDesc(MemStream& stream, VFSFile& file, size_t* baseIdx) {
    file.flags = stream.ReadU16();

    if (mIsLastLight) {
        //#NOTE_SK: in Last Light folder flag is 16 ... make it 8 so it's compatible with later VFX versions
        file.flags >>= 1;
    }

    if (TestBit(file.flags, VFSFile::Flag_Folder)) {
        file.folder.numFiles = stream.ReadU16();
        file.folder.firstFileIdx = stream.ReadU32();
        file.folder.fileLink = 0;
        file.folder.layerLink = kBaseLayer;
    } else {
        file.file.packIdx = stream.ReadU16();
        file.file.offset = stream.ReadU32();
        file.file.realLen = stream.ReadU32();
        file.file.packedLen = stream.ReadU32();
        file.file.duplicates = kInvalidValue;

        if (baseIdx) {
            *baseIdx = stream.ReadU32();
        }
    }

    if (!baseIdx) {
        this->ReadProtectedName(stream, file.name);
    }
}

void MetroVFS::ReadProtectedName(MemStream& stream, CharString& name) {
    const uint16_t stringHeader = stream.ReadU16();
    const size_t stringLen = (stringHeader & 0xFF);
    const char xorMask = scast<char>((stringHeader >> 8) & 0xFF);

    name.reserve(stringLen);
    for (size_t i = 1; i < stringLen; ++i) {
        const char ch = stream.ReadTyped<char>();
        name.push_back(ch ^ xorMask);
    }

    stream.ReadTyped<char>(); // terminating null
}

bool MetroVFS::LoadContent(const fs::path& filePath) {
    bool result = false;

    std::ifstream vfxFile(filePath, std::ifstream::binary);
    if (vfxFile.good()) {
        BytesArray fileData;

        vfxFile.seekg(0, std::ios::end);
        fileData.resize(vfxFile.tellg());
        vfxFile.seekg(0, std::ios::beg);
        vfxFile.read(rcast<char*>(fileData.data()), fileData.size());
        vfxFile.close();

        MemStream stream(fileData.data(), fileData.size());

        if (this->LoadRegistryHeader(stream)) {
            auto& baseFilesArray = mFiles[kBaseLayer];

            baseFilesArray.resize(mNumFiles);
            for (VFSFile& file : baseFilesArray) {
                this->ReadFileDesc(stream, file, nullptr);
            }

            mDuplicates.resize(mNumDuplicates);
            size_t dupIdx = 0;
            for (VFSFile& dup : mDuplicates) {
                size_t baseIdx = kInvalidValue;
                this->ReadFileDesc(stream, dup, &baseIdx);

                VFSFile& baseFile = baseFilesArray[baseIdx];
                dup.file.duplicates = baseFile.file.duplicates;
                baseFile.file.duplicates = dupIdx;

                ++dupIdx;
            }

            result = true;
        }
    }

    return result;
}

bool MetroVFS::LoadPatch(const fs::path& filePath, const size_t layer) {
    bool result = false;

    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        BytesArray fileData;

        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        MemStream stream(fileData.data(), fileData.size());

        const size_t packagesOffset = mPackages.size();
        if (this->LoadRegistryHeader(stream)) {
            auto& filesArray = mFiles[layer];

            size_t fileIdx = 0;
            for (VFSFile& patchFile : filesArray) {
                this->ReadFileDesc(stream, patchFile, nullptr);

                if ((patchFile.flags & (VFSFile::Flag_Folder | VFSFile::Flag_New)) == VFSFile::Flag_Folder) {
                    // this means that this folderis not new, so we should find the original one and update
                    VFSFile* originalFolder = this->FindByName(patchFile.name);
                    assert(originalFolder != nullptr);

                    if (originalFolder->folder.layerLink && !TestBit(originalFolder->flags, VFSFile::Flag_New)) {
                        originalFolder = this->FindTail(&mFiles[originalFolder->folder.layerLink][originalFolder->folder.fileLink]);
                    }

                    originalFolder->flags = RemoveBit(originalFolder->flags, VFSFile::Flag_New);
                    originalFolder->folder.layerLink = layer;
                    originalFolder->folder.fileLink = fileIdx;

                    patchFile.name = originalFolder->name; // am I sure ???
                } else {
                    if (TestBit(patchFile.flags, VFSFile::Flag_New)) {
                        patchFile.folder.layerLink = layer;
                    }
                }

                patchFile.file.packIdx += packagesOffset;

                ++fileIdx;
            }
        }

        result = true;
    }

    return result;
}

// find functions, should mimic same as in Metro engine
MetroVFS::VFSFile* MetroVFS::Find(const size_t layer, MetroVFS::VFSFile* current, const StringView& path, const size_t nearest, size_t* _layer) {
    assert(TestBit(current->flags, VFSFile::Flag_Folder));

    VFSFile* result = nullptr;

    if (current->folder.layerLink != kBaseLayer && !TestBit(current->flags, VFSFile::Flag_New)) {
        VFSFile* newCurrent = &mFiles[current->folder.layerLink][current->folder.fileLink];

        size_t testLayer = _layer ? current->folder.layerLink : 0;
        result = this->Find(current->folder.layerLink, newCurrent, path, nearest, &testLayer);
        if (result && _layer) {
            *_layer = testLayer;
        }
    }

    if (!result) {
        const size_t slashPos = path.find_first_of('\\');
        StringView nameToSearch = (slashPos != StringView::npos) ? path.substr(0, slashPos) : path;

        auto& filesArray = mFiles[layer];
        VFSFile* foundFile = nullptr;
        for (size_t idx = current->folder.firstFileIdx, endIdx = current->folder.firstFileIdx + current->folder.numFiles; idx < endIdx; ++idx) {
            if (filesArray[idx].name == nameToSearch) {
                foundFile = &filesArray[idx];
                break;
            }
        }

        if (foundFile) {
            if (slashPos != StringView::npos) { // go to next folder
                result = this->Find(layer, foundFile, path.substr(slashPos + 1), nearest, _layer);
            } else {
                result = this->FindNearest(layer, foundFile, nearest);
            }
        }
    }

    return result;
}

MetroVFS::VFSFile* MetroVFS::FindNearest(const size_t /*layer*/, MetroVFS::VFSFile* current, const size_t nearest) {
    //vfs::package_registry *v4; // rbx
    //unsigned int v5; // er9
    //vfs::package_registry::file *v6; // r10
    //vfs::package_registry::file *i; // rcx
    //unsigned int v8; // eax
    //unsigned int v9; // edx
    //unsigned int v10; // eax
    //unsigned int v11; // eax

    //v4 = this;
    //if (nearest) {
    //    v5 = -1;
    //    v6 = (vfs::package_registry::file *)((char *)this + 8 * layer);// this->files[layer].data()
    //    for (i = F; i; i = (vfs::package_registry::file *)((char *)v4->duplicates._array + 48 * v11)) {
    //        v8 = i->ptr;
    //        if (LODWORD(i->package) == LODWORD(v6[3].name.p_)) {
    //            v9 = HIDWORD(v6[3].name.p_);
    //            if (v8 <= v9)
    //                v10 = v9 - v8;
    //            else
    //                v10 = v8 - v9;
    //            if (v10 < v5) {
    //                v5 = v10;
    //                F = i;
    //            }
    //        }
    //        v11 = i->duplicates;
    //        if (v11 == -1)
    //            break;
    //    }
    //    LODWORD(v6[3].name.p_) = F->package;
    //    HIDWORD(v6[3].name.p_) = LODWORD(F->size_compressed) + F->ptr;
    //}
    //return F;

    if (!nearest) {
        return current;
    } else {
        //TODO_SK:
        return current;
    }
}

MetroVFS::VFSFile* MetroVFS::FindTail(MetroVFS::VFSFile* file) {
    for (size_t i = file->folder.layerLink; i != kBaseLayer; i = file->folder.layerLink) {
        if (TestBit(file->flags, VFSFile::Flag_New)) {
            break;
        }
        file = &mFiles[i][file->folder.fileLink];
    }
    return file;
}
