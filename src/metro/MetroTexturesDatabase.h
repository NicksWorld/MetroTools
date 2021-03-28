#pragma once
#include "mycommon.h"
#include "mymath.h"
#include "MetroTypes.h"

class MetroTexturesDBImpl;

struct MetroTextureInfoCommon {
    // 2033
    int32_t     type;
    bool        animated;
    uint32_t    fmt;
    uint32_t    r_width;
    uint32_t    r_height;
    CharString  name;
    CharString  bump_name;
    float       bump_height;
    uint8_t     parr_height;
    CharString  det_name;
    float       det_u_scale;
    float       det_v_scale;
    float       det_int;
    bool        mip_enabled;
    bool        streamable;
    bool        priority;
    uint32_t    avg_color;
};

class MetroTexturesDatabase {
public:
    MetroTexturesDatabase();
    ~MetroTexturesDatabase();

    bool                    Initialize(const MetroGameVersion version, const fs::path& binPath = fs::path());
    bool                    Good() const;
    bool                    SaveBin(const fs::path& binPath);
    void                    Shutdown();

    const CharString&       GetSourceName(const HashString& name) const;
    const CharString&       GetBumpName(const HashString& name) const;
    const CharString&       GetDetName(const HashString& name) const;
    const CharString&       GetAuxName(const HashString& name, const size_t idx) const;
    StringArray             GetAllLevels(const HashString& name) const;
    bool                    IsAlbedo(const MyHandle file) const;
    MetroSurfaceDescription GetSurfaceSetFromFile(const MyHandle file, const bool allMips) const;
    MetroSurfaceDescription GetSurfaceSetFromName(const HashString& textureName, const bool allMips) const;

    size_t                  GetNumTextures() const;
    const CharString&       GetTextureNameByIdx(const size_t idx) const;
    void                    FillCommonInfoByIdx(const size_t idx, MetroTextureInfoCommon& info) const;
    void                    SetCommonInfoByIdx(const size_t idx, const MetroTextureInfoCommon& info);
    void                    RemoveTextureByIdx(const size_t idx);
    void                    AddTexture(const MetroTextureInfoCommon& info);


private:
    MetroTexturesDBImpl*    mImpl;
};
