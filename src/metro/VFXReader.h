#pragma once
#include "MetroTypes.h"

struct Package {
    CharString      name;
    StringArray     levels;
    size_t          chunk;
};

class VFXReader {
public:
    // Known guids
    static inline MetroGuid kGUIDLastLightSteam = { 0xE4727F4A, 0xF56A, 0x4998, 0x2E84, { 0xFC, 0xD2, 0x75, 0x22, 0xEB, 0x3D } };
    static inline MetroGuid kGUIDLastLightSteamLinux = { 0xB03B72E1, 0x7659, 0x4D74, 0x618C, { 0xBF, 0x1F, 0xBD, 0x55, 0x22, 0x1B } };
    static inline MetroGuid kGUIDLastLightSteamMacOS = { 0x3541E232, 0x1A41, 0x4B20, 0xAC94, { 0x86, 0x43, 0x31, 0xC2, 0x4C, 0x41 } };
    static inline MetroGuid kGUIDLastLightXBox360 = { 0x96B68062, 0x41B8, 0x41EE, 0xEAB2, { 0x90, 0xED, 0xF0, 0x78, 0xF2, 0x22 } };
    static inline MetroGuid kGUIDLastLightXBox360_Build_Oct18_2012 = { 0x292A1047, 0x06A4, 0x4423, 0xE980, { 0xB3, 0xEA, 0x49, 0x32, 0xB0, 0x31 } };
    static inline MetroGuid kGUIDLastLightXBox360_Build_Dec03_2012 = { 0xA328D21D, 0x8D8C, 0x477D, 0x4AA8, { 0xFD, 0xDA, 0x86, 0x59, 0xEF, 0x62 } };

    static inline MetroGuid kGUIDRedux2033PC = { 0x3CC39580, 0x3AD4, 0x4007, 0xEDBF, { 0xC1, 0xB5, 0xE4, 0x4D, 0x20, 0xCB } };
    static inline MetroGuid kGUIDReduxLastLightPC = { 0xF48EB978, 0xAF7E, 0x42B0, 0x37BF, { 0xD2, 0x09, 0x7A, 0x92, 0xAF, 0x9A } };
    static inline MetroGuid kGUIDRedux2033Switch = { 0xEA0296FA, 0xB4B1, 0x4C31, 0x7580, { 0xC2, 0x03, 0xF0, 0xFC, 0xBB, 0x74 } };
    static inline MetroGuid kGUIDReduxLastLightSwitch = { 0xE983FCA4, 0x3EC9, 0x4BBD, 0x7FBE, { 0x56, 0xA9, 0x53, 0x10, 0xD0, 0xB5 } };

    static inline MetroGuid kGUIDArktika1_1 = { 0xA4B0F5D3, 0x875C, 0x481C, 0x1B9C, { 0x2D, 0xB9, 0xC7, 0xC4, 0x0B, 0xD5 } };
    static inline MetroGuid kGUIDArktika1_2 = { 0xE113CED0, 0x8B8C, 0x4E59, 0x7480, { 0x1B, 0x32, 0x0F, 0x13, 0xD2, 0x2B } };

    static inline MetroGuid kGUIDExodus = { 0x9FE25B12, 0xF276, 0x40F4, 0xEAB8, { 0x0F, 0xE1, 0xA4, 0xC6, 0x9E, 0x7A } };

    static bool IsGUIDLastLight(const MetroGuid& guid);

    static const size_t kVFXVersionUnknown      = 0;
    static const size_t kVFXVersion2033Redux    = 1;
    static const size_t kVFXVersionArktika1     = 2;
    static const size_t kVFXVersionExodus       = 3;
    static const size_t kVFXVersionMax          = 4;

public:
    VFXReader();
    ~VFXReader();

public:
    bool                        LoadFromFile(const fs::path& filePath);
    bool                        SaveToFile(const fs::path& filePath) const;
    void                        Close();

    const CharString&           GetContentVersion() const;
    size_t                      GetVersion() const;
    const MetroGuid&            GetGUID() const;

    MemStream                   ExtractFile(const size_t fileIdx, const size_t subOffset = kInvalidValue, const size_t subLength = kInvalidValue) const;

    bool                        Good() const;

    const CharString&           GetSelfName() const;
    const fs::path&             GetAbsolutePath() const;

    const MyArray<Package>&     GetAllPacks() const;
    const MyArray<MetroFile>&   GetAllFiles() const;
    const MyArray<size_t>&      GetAllFolders() const;

    const MetroFile&            GetRootFolder() const;
    const MetroFile&            GetFile(const size_t idx) const;

    // modification
    void                        AddPackage(const Package& pak);
    void                        ReplaceFileInfo(const size_t idx, const MetroFile& newFile);
    void                        AppendFolder(const MetroFile& folder);

private:
    void                        ReadFileDescription(MetroFile& mf, MemStream& stream, const bool isDuplicate, const bool isLastLight);

private:
    size_t                      mVersion;
    size_t                      mCompressionType;
    bool                        mIsLastLight;
    CharString                  mContentVersion;
    MetroGuid                   mGUID;
    CharString                  mFileName;
    fs::path                    mBasePath;
    fs::path                    mAbsolutePath;
    MyArray<Package>            mPaks;
    MyArray<MetroFile>          mFiles;
    MyArray<size_t>             mFolders;
    MyArray<MetroFile>          mDuplicates;
};
