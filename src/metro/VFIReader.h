#pragma once
#include "MetroTypes.h"

class VFIReader {
public:
    VFIReader();
    ~VFIReader();

    bool                    LoadFromFile(const fs::path& filePath);
    bool                    LoadFromUPK(const fs::path& filePath);
    void                    Close();

    size_t                  GetVersion() const;
    const MetroGuid&        GetGUID() const;
    const CharString&       GetSelfName() const;
    const fs::path&         GetAbsolutePath() const;

    size_t                  GetRootFolderIdx() const;
    bool                    IsFolder(const size_t idx) const;
    const CharString&       GetFileName(const size_t idx) const;
    size_t                  GetSizeUncompressed(const size_t idx) const;
    size_t                  GetSizeCompressed(const size_t idx) const;
    const MyArray<size_t>&  GetChildren(const size_t idx) const;

    MemStream               ExtractFile(const size_t fileIdx, const size_t subOffset = kInvalidValue, const size_t subLength = kInvalidValue) const;

private:
    void                    ReadPackage(MemStream& stream);
    void                    BuildFileTree();
    size_t                  GetOrAddFolder(const HashString& folderName, const size_t parentFolder);

private:
    struct Package {
        CharString      name;
        size_t          size;
    };

    struct File {
        size_t          packIdx;
        size_t          offset;
        size_t          sizeUncompressed;
        size_t          sizeCompressed;
        CharString      fullPath;
        HashString      name;
        MyArray<size_t> children;

        inline bool IsFolder() const {
            return this->sizeUncompressed == 0;
        }
    };

private:
    size_t              mVersion;
    MetroGuid           mGUID;
    CharString          mFileName;
    fs::path            mBasePath;
    fs::path            mAbsolutePath;
    MyArray<Package>    mPackages;
    MyArray<File>       mFiles;
    size_t              mRootFolderIdx;
};
