#pragma once
#include "metro/MetroTypes.h"

class MetroReflectionStream;

class MetroSession {
public:
    MetroSession();
    ~MetroSession();

    void                Serialize(MetroReflectionStream& s);

    void                SetGameVersion(const MetroGameVersion version);
    void                SetContentFolder(const fs::path& folder);

    MetroGameVersion    GetGameVersion() const;
    const fs::path&     GetContentFolder() const;

private:
    MetroGameVersion    mGameVersion;
    fs::path            mContentFolder;
};

class MetroSessionsList {
public:
    MetroSessionsList();
    ~MetroSessionsList();

    bool                    LoadFromFile(const fs::path& filePath);
    bool                    SaveToFile(const fs::path& filePath);

    void                    Serialize(MetroReflectionStream& s);

    void                    AddSession(const MetroSession& session);
    size_t                  GetNumSessions() const;
    const MetroSession&     GetSession(const size_t idx) const;

private:
    MyArray<MetroSession>   sessions;
};
