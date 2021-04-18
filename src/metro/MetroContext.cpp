#include "MetroContext.h"
#include "VFXReader.h"


MetroContext::MetroContext()
    : mGameVersion(MetroGameVersion::Unknown)
{
}
MetroContext::~MetroContext() {
}

bool MetroContext::InitFromGameFolder(const fs::path& folderPath) {
    bool result = false;

    this->Shutdown();

    MetroFileSystem& mfs = this->GetFilesystem();
    if (mfs.InitFromGameFolder(folderPath)) {
        this->GuessGameVersionFromFS();
        mGameFolder = folderPath;
        this->InitPrivate();

        result = true;
    }

    return result;
}

bool MetroContext::InitFromContentFolder(const fs::path& folderPath) {
    bool result = false;

    this->Shutdown();

    MetroFileSystem& mfs = this->GetFilesystem();
    if (mfs.InitFromContentFolder(folderPath)) {
        mGameVersion = MetroGameVersion::Redux;
        mGameFolder = folderPath.parent_path();
        this->InitPrivate();
    }

    return result;
}

bool MetroContext::InitFromSingleArchive(const fs::path& archivePath) {
    bool result = false;

    this->Shutdown();

    CharString extension = archivePath.extension().string();

    MetroFileSystem& mfs = this->GetFilesystem();

    result = (extension == ".vfi") ? mfs.InitFromSingleVFI(archivePath) : mfs.InitFromSingleVFX(archivePath);

    if (result) {
        this->GuessGameVersionFromFS();
        mGameFolder = archivePath.parent_path();
        this->InitPrivate();
    }

    return result;
}

void MetroContext::Shutdown() {
    this->GetFilesystem().Shutdown();
    this->GetTexturesDB().Shutdown();
    this->GetConfigsDB().Shutdown();

    for (size_t i = scast<size_t>(MetroLanguage::First); i <= scast<size_t>(MetroLanguage::Last); ++i) {
        this->GetFontsDB(scast<MetroLanguage>(i)).Shutdown();
    }
}

MetroGameVersion MetroContext::GetGameVersion() const {
    return mGameVersion;
}

void MetroContext::SetGameVersion(const MetroGameVersion version) {
    mGameVersion = version;
}

MetroTypedStrings& MetroContext::GetTypedStrings() {
    return MetroTypedStrings::Get();
}

MetroFileSystem& MetroContext::GetFilesystem() {
    return MetroFileSystem::Get();
}

MetroTexturesDatabase& MetroContext::GetTexturesDB() {
    return mTexturesDB;
}

MetroConfigsDatabase& MetroContext::GetConfigsDB() {
    return MetroConfigsDatabase::Get();
}

MetroFontsDatabase& MetroContext::GetFontsDB(const MetroLanguage lng) {
    return MetroFontsDatabase::Get(lng);
}

const CharString& MetroContext::GetSkeletonExtension() const {
    static const CharString kSkelExt2033 = ".skeleton";
    static const CharString kSkelExt = ".skeleton.bin";

    if (mGameVersion == MetroGameVersion::OG2033) {
        return kSkelExt2033;
    } else {
        return kSkelExt;
    }
}

const CharString& MetroContext::GetMotionExtension() const {
    static const CharString kMotionExt2033 = ".motion";
    static const CharString kMotionExt = ".m2"; // though in LL there's .m3 files, but seems like they're duplicating .m2 so we don't care

    if (mGameVersion == MetroGameVersion::OG2033) {
        return kMotionExt2033;
    } else {
        return kMotionExt;
    }
}

const CharString& MetroContext::GetClothModelExtension() const {
    static const CharString kLegacyClothExt = ".sftmdl_pc";
    static const CharString kReduxClothExt = ".sftmdl331";
    static const CharString kClothExt = ".sftmdl332";

    if (mGameVersion == MetroGameVersion::OG2033 || mGameVersion == MetroGameVersion::OGLastLight) {
        return kLegacyClothExt;
    } else if (mGameVersion == MetroGameVersion::Redux) {
        return kReduxClothExt;
    } else {
        return kClothExt;
    }
}



void MetroContext::GuessGameVersionFromFS() {
    MetroFileSystem& mfs = this->GetFilesystem();

    if (mfs.IsMetro2033()) {
        mGameVersion = MetroGameVersion::OG2033;
    } else {
        const size_t archiveVersion = mfs.GetVFXVersion();
        const MetroGuid& guid = mfs.GetVFXGUID();

        if (archiveVersion >= VFXReader::kVFXVersionExodus) {
            mGameVersion = MetroGameVersion::Exodus;
        } else if (archiveVersion == VFXReader::kVFXVersionArktika1) {  // ???
            mGameVersion = MetroGameVersion::Arktika1;
        } else if (archiveVersion == VFXReader::kVFXVersion2033Redux) {
            //#NOTE_SK: why in the fuck Last Light, Redux and Arktika.1 all have the same VFX version ?
            if (guid == VFXReader::kGUIDArktika1_1 || guid == VFXReader::kGUIDArktika1_2) {
                mGameVersion = MetroGameVersion::Arktika1;
            } else if (VFXReader::IsGUIDLastLight(guid)) {
                mGameVersion = MetroGameVersion::OGLastLight;
            } else {
                mGameVersion = MetroGameVersion::Redux;
            }
        }
    }
}

//#TODO_SK: multithread this!
void MetroContext::InitPrivate() {
    LogPrint(LogLevel::Info, "Loading typed_strings.bin from " + mGameFolder.u8string());
    if (!MetroTypedStrings::Get().Initialize(mGameFolder)) {
        LogPrint(LogLevel::Error, "Failed to load typed_strings.bin !");
    }

    const MetroFileSystem& mfs = this->GetFilesystem();

    this->GetTexturesDB().Initialize(mGameVersion);

    // Load config.bin
    MemStream stream = mfs.OpenFileFromPath(R"(content\config.bin)");
    if (stream) {
        MetroConfigsDatabase::Get().LoadFromData(stream);
    }

    // load fonts
    if (MetroConfigsDatabase::Get().GetNumFiles() > 0) {
        for (size_t i = scast<size_t>(MetroLanguage::First); i <= scast<size_t>(MetroLanguage::Last); ++i) {
            CharString binName = MetroFontsDatabase::MakeFontDBPath(scast<MetroLanguage>(i));
            MemStream stream = MetroConfigsDatabase::Get().GetFileStream(binName);
            if (stream) {
                MetroFontsDatabase::Get(scast<MetroLanguage>(i)).LoadFromData(stream);
            }
        }
    }

    MetroWeaponry::Get().Initialize();

    // load materials database
    //if (cfgDb) {
    //    MemStream materialsStream = cfgDb->GetFileStream(R"(content\scripts\materials.bin)");
    //    if (materialsStream.Good()) {
    //        MetroMaterialsDatabase materialsDb;
    //        materialsDb.LoadFromData(materialsStream);
    //    }
    //}
}
