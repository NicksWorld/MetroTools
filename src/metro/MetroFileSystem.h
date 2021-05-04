#pragma once
#include "MetroTypes.h"

class VFIReader;
class VFXReader;

class MetroFileSystem {
    IMPL_SINGLETON(MetroFileSystem)

public:
    struct Paths {
        static CharString MotionsFolder;
        static CharString MeshesFolder;
        static CharString LocalizationsFolder;
        static CharString TexturesFolder;
        static CharString WeaponryFolder;
    };

    struct MetroFSEntry {
        HashString  name;
        size_t      idx;
        MyHandle    parent;
        size_t      firstChild;
        size_t      nextSibling;
        size_t      archIdx;
        size_t      fileIdx;

        // dups stuff
        size_t      dupIdx;
    };

protected:
    MetroFileSystem();
    ~MetroFileSystem();

public:
    bool                    InitFromGameFolder(const fs::path& gameFolder);
    bool                    InitFromContentFolder(const fs::path& gameFolder);
    bool                    InitFromSingleVFX(const fs::path& vfxPath);
    bool                    InitFromSingleVFI(const fs::path& vfiPath);
    void                    Shutdown();
    bool                    Empty() const;
    bool                    IsSingleArchive() const;
    const CharString&       GetArchiveName(const size_t idx) const;
    size_t                  GetVFXVersion() const;
    const MetroGuid&        GetVFXGUID() const;
    bool                    IsMetro2033() const;

    MetroFSPath             GetRootFolder() const;

    bool                    IsFolder(const MetroFSPath& entry) const;
    bool                    IsFile(const MetroFSPath& entry) const;
    CharString              GetName(const MetroFSPath& entry) const;
    CharString              GetFullPath(const MetroFSPath& entry) const;
    size_t                  GetCompressedSize(const MetroFSPath& entry) const;
    size_t                  GetUncompressedSize(const MetroFSPath& entry) const;

    size_t                  CountFilesInFolder(const MetroFSPath& entry, const bool recursive) const;

    MetroFSPath             GetParentFolder(const MetroFSPath& entry) const;
    MetroFSPath             FindFile(const CharString& fileName, const MetroFSPath& inFolder = MetroFSPath(MetroFSPath::Invalid)) const;
    MetroFSPath             FindFolder(const CharString& folderPath, const MetroFSPath& inFolder = MetroFSPath(MetroFSPath::Invalid)) const;
    MyArray<MetroFSPath>    FindFilesInFolder(const CharString& folder, const CharString& extension, const bool withSubfolders = true) const;
    MyArray<MetroFSPath>    FindFilesInFolder(const MetroFSPath& folder, const CharString& extension, const bool withSubfolders = true) const;

    MyHandle                GetFirstChild(const MyHandle parentEntry) const;
    MyHandle                GetNextChild(const MyHandle currentChild) const;
    MyHandle                FindChild(const MyHandle parentEntry, const HashString& childName) const;

    MemStream               OpenFileStream(const MetroFSPath& entry, const size_t subOffset = kInvalidValue, const size_t subLength = kInvalidValue) const;
    MemStream               OpenFileFromPath(const CharString& fileName) const;

private:
    bool                    AddVFX(const fs::path& vfxPath);
    bool                    AddVFI(const fs::path& vfiPath);
    bool                    AddUPK(const fs::path& upkPath);
    void                    MergeFolderRecursive(MyHandle parentEntry, const MetroFile& folder, const VFXReader& vfxReader);
    void                    MergeFolderRecursive(MyHandle parentEntry, const size_t folder, const VFIReader& vfiReader);
    MyHandle                AddEntryFolder(const MyHandle parentEntry, const HashString& name);
    MyHandle                AddEntryFile(const MyHandle parentEntry, const MetroFile& file);
    MyHandle                AddEntryCommon(const MyHandle parentEntry, const MetroFSEntry& entry);

    fs::path                MakeProperFullPath(const fs::path& filePath) const;

private:
    bool                    mIsMetro2033FS;
    MyArray<VFIReader*>     mLoadedVFI;
    MyArray<VFXReader*>     mLoadedVFX;
    MyArray<MetroFSEntry>   mEntries;
    MyArray<MetroFSEntry>   mDupEntries;
    size_t                  mCurrentArchIdx;

    // real fs
    bool                    mIsRealFS;
    fs::path                mRealFSRoot;
};
