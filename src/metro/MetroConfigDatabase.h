#pragma once
#include "mycommon.h"

class MetroConfigsDatabase {
    IMPL_SINGLETON(MetroConfigsDatabase)

public:
    struct ConfigInfo {
        size_t      idx;
        uint32_t    nameCRC;
        CharString  nameStr;
        size_t      offset;
        size_t      length;
    };

protected:
    MetroConfigsDatabase();
    ~MetroConfigsDatabase();

public:
    bool                LoadFromData(MemStream& stream);
    void                Shutdown();

    const ConfigInfo*   FindFile(const uint32_t nameCRC) const;
    const ConfigInfo*   FindFile(const CharString& name) const;

    MemStream           GetFileStream(const CharString& name) const;
    bool                ReplaceFileByIdx(const size_t chunkIdx, const MemStream& stream);

    size_t              GetNumFiles() const;
    const ConfigInfo&   GetFileByIdx(const size_t chunkIdx) const;

    const MemStream&    GetDataStream() const;
    void                SetDirty(const bool dirty);
    bool                IsDirty() const;

private:
    MemStream           mStream;
    MyArray<ConfigInfo> mConfigsChunks;
    bool                mIsDirty;

    size_t              mStatsTotalDecryptedNames;
    size_t              mStatsTotalEncryptedNames;
};
