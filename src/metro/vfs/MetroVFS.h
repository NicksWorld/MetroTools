#pragma once
#include "mycommon.h"
#include "metro/MetroTypes.h"

class MetroVFS {
public:
    static const size_t kMaxLayers = 16;
    static const size_t kBaseLayer = 0;

    struct VFSFile {
        static const size_t Flag_Folder = 8;
        static const size_t Flag_New    = 4;    // patches

        CharString      name;
        size_t          flags;
        union {
            struct FileFields {
                size_t  packIdx;
                size_t  offset;
                size_t  realLen;
                size_t  packedLen;
                size_t  duplicates;
            } file;

            struct FolderFields {
                size_t  _;  // stub to not interfere with packIdx in file
                size_t  numFiles;
                size_t  firstFileIdx;
                size_t  fileLink;
                size_t  layerLink;
            } folder;
        };
    };

    struct VFSPackage {
        CharString  name;
        StringArray levels;
        size_t      size;
    };

public:
    MetroVFS();
    ~MetroVFS();

    bool        LoadGameFolder(const fs::path& gameFolder);
    bool        LoadSingleRegistry(const fs::path& filePath);

    VFSFile*    FindByName(const StringView& path);

private:
    bool        LoadRegistryHeader(MemStream& stream);
    void        ReadFileDesc(MemStream& stream, VFSFile& file, size_t* baseIdx);
    void        ReadProtectedName(MemStream& stream, CharString& name);
    bool        LoadContent(const fs::path& filePath);
    bool        LoadPatch(const fs::path& filePath, const size_t layer);

    // find functions, should mimic same as in Metro engine
    VFSFile*    Find(const size_t layer, VFSFile* current, const StringView& path, const size_t nearest, size_t* _layer);
    VFSFile*    FindNearest(const size_t layer, VFSFile* current, const size_t nearest);
    VFSFile*    FindTail(VFSFile* file);

private:
    // transient fields
    size_t              mVersion;
    size_t              mCompressionType;
    CharString          mContentVersion;
    MetroGuid           mGuid;
    size_t              mNumPackages;
    size_t              mNumFiles;
    size_t              mNumDuplicates;

    // hack for Last Light files
    bool                mIsLastLight;

    // normal fields
    size_t              mLayersUsed;
    MyArray<VFSPackage> mPackages;
    MyArray<VFSFile>    mFiles[kMaxLayers];
    MyArray<VFSFile>    mDuplicates;
};
