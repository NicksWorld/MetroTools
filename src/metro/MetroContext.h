#pragma once
#include "MetroTypes.h"
#include "MetroFileSystem.h"
#include "MetroTexturesDatabase.h"
#include "MetroConfigDatabase.h"
#include "MetroFonts.h"
#include "MetroTypedStrings.h"
#include "MetroWeaponry.h"

class MetroContext {
    IMPL_SINGLETON(MetroContext)

protected:
    MetroContext();
    ~MetroContext();

public:
    bool                    InitFromGameFolder(const fs::path& folderPath);
    bool                    InitFromContentFolder(const fs::path& folderPath);
    bool                    InitFromSingleArchive(const fs::path& archivePath);
    void                    Shutdown();

    MetroGameVersion        GetGameVersion() const;
    void                    SetGameVersion(const MetroGameVersion version);

    MetroTypedStrings&      GetTypedStrings();
    MetroFileSystem&        GetFilesystem();
    MetroTexturesDatabase&  GetTexturesDB();
    MetroConfigsDatabase&   GetConfigsDB();
    MetroFontsDatabase&     GetFontsDB(const MetroLanguage lng);

    const CharString&       GetSkeletonExtension() const;
    const CharString&       GetMotionExtension() const;
    const CharString&       GetClothModelExtension() const;

private:
    void                    GuessGameVersionFromFS();
    void                    InitPrivate();

private:
    fs::path                mGameFolder;
    MetroGameVersion        mGameVersion;
    MetroTexturesDatabase   mTexturesDB;
};
