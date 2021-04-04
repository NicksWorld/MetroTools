#include "MetroTexturesDatabase.h"
#include "MetroTypes.h"
#include "MetroBinArrayArchive.h"
#include "MetroBinArchive.h"
#include "MetroContext.h"
#include "reflection/MetroReflection.h"

#include "log.h"

//#NOTE_SK: because we're in CLR mode, we can't use C++ threads, and have to use old shit :(
//#include <thread>
//#include <functional>
#include <windows.h>
#include <process.h>

class MetroTexturesDBImpl {
public:
    MetroTexturesDBImpl() {}
    virtual ~MetroTexturesDBImpl() {}

    virtual bool                    Initialize(const fs::path& binPath) = 0;
    virtual bool                    SaveBin(const fs::path& /*binPath*/) { return false; }

    virtual const CharString&       GetSourceName(const HashString& name) const = 0;
    virtual const CharString&       GetBumpName(const HashString& name) const = 0;
    virtual const CharString&       GetDetName(const HashString& name) const = 0;
    virtual const CharString&       GetAuxName(const HashString& name, const size_t idx) const = 0;
    virtual StringArray             GetAllLevels(const HashString& name) const = 0;
    virtual bool                    IsAlbedo(const MetroFSPath& file) const = 0;
    virtual MetroSurfaceDescription GetSurfaceSetFromFile(const MetroFSPath& file, const bool allMips) const = 0;
    virtual MetroSurfaceDescription GetSurfaceSetFromName(const HashString& textureName, const bool allMips) const = 0;

    virtual size_t                  GetNumTextures() const { return 0; }
    virtual const CharString&       GetTextureNameByIdx(const size_t /*idx*/) const { return kEmptyString; }
    virtual void                    FillCommonInfoByIdx(const size_t /*idx*/, MetroTextureInfoCommon& /*info*/) const { }
    virtual void                    SetCommonInfoByIdx(const size_t /*idx*/, const MetroTextureInfoCommon& /*info*/) { }
    virtual void                    RemoveTextureByIdx(const size_t /*idx*/) { }
    virtual void                    AddTexture(const MetroTextureInfoCommon& /*info*/) { }
};

// common stuff
enum class MetroTextureType : uint8_t {
    Diffuse             = 0,
    Detail_diffuse      = 1,
    Cubemap             = 2,
    Cubemap_hdr         = 3,        //#NOTE_SK: has sph_coefs (fp32_array), seems to be SH1
    Terrain             = 4,
    Bumpmap             = 5,
    Diffuse_va          = 6,
    Arbitrary4          = 7,
    Normalmap           = 8,
    Normalmap_alpha     = 9,
    Normalmap_detail    = 10,
    Unknown_01          = 11,
    Unknown_has_lum     = 12,       //#NOTE_SK: has lum (u8_array)
    Instance            = 64
};

enum class MetroTextureDisplType : uint8_t {
    Simple      = 0,
    Parallax    = 1,
    Displace    = 2
};

struct MetroTextureAliasInfo {
    CharString  src;
    CharString  dst;

    void Serialize(MetroReflectionStream& s) {
        METRO_SERIALIZE_MEMBER(s, src);
        METRO_SERIALIZE_MEMBER(s, dst);
    }
};

#include "metro/textures/MetroTexturesDBImpl2033.inl"
#include "metro/textures/MetroTexturesDBImplLastLight.inl"
#include "metro/textures/MetroTexturesDBImplRedux.inl"
#include "metro/textures/MetroTexturesDBImplArktika1.inl"
#include "metro/textures/MetroTexturesDBImplExodus.inl"

MetroTexturesDatabase::MetroTexturesDatabase()
    : mImpl(nullptr) {
}
MetroTexturesDatabase::~MetroTexturesDatabase() {
    this->Shutdown();
}

bool MetroTexturesDatabase::Initialize(const MetroGameVersion version, const fs::path& binPath) {
    bool result = false;

    this->Shutdown();

    switch (version) {
        case MetroGameVersion::OG2033:
            mImpl = new Metro2033Impl::MetroTexturesDBImpl2033();
        break;
        case MetroGameVersion::OGLastLight:
            mImpl = new LastLightImpl::MetroTexturesDBImplLastLight();
        break;
        case MetroGameVersion::Redux:
            mImpl = new ReduxImpl::MetroTexturesDBImplRedux();
        break;
        case MetroGameVersion::Arktika1:
            mImpl = new Arktika1Impl::MetroTexturesDBImplArktika1();
        break;
        case MetroGameVersion::Exodus:
            mImpl = new ExodusImpl::MetroTexturesDBImplExodus();
        break;
        default:
            mImpl = nullptr;
            break;
    }

    if (mImpl) {
        if (mImpl->Initialize(binPath)) {
            result = true;
        } else {
            delete mImpl;
            mImpl = nullptr;
        }
    }

    return result;
}

bool MetroTexturesDatabase::Good() const {
    return mImpl != nullptr;
}

bool MetroTexturesDatabase::SaveBin(const fs::path& binPath) {
    if (this->Good()) {
        return mImpl->SaveBin(binPath);
    } else {
        return false;
    }
}

void MetroTexturesDatabase::Shutdown() {
    MySafeDelete(mImpl);
}

const CharString& MetroTexturesDatabase::GetSourceName(const HashString& name) const {
    return this->Good() ? mImpl->GetSourceName(name) : kEmptyString;
}

const CharString& MetroTexturesDatabase::GetBumpName(const HashString& name) const {
    return this->Good() ? mImpl->GetBumpName(name) : kEmptyString;
}

const CharString& MetroTexturesDatabase::GetDetName(const HashString& name) const {
    return this->Good() ? mImpl->GetDetName(name) : kEmptyString;
}

const CharString& MetroTexturesDatabase::GetAuxName(const HashString& name, const size_t idx) const {
    return this->Good() ? mImpl->GetAuxName(name, idx) : kEmptyString;
}

StringArray MetroTexturesDatabase::GetAllLevels(const HashString& name) const {
    return this->Good() ? mImpl->GetAllLevels(name) : StringArray();
}

bool MetroTexturesDatabase::IsAlbedo(const MetroFSPath& file) const {
    return this->Good() ? mImpl->IsAlbedo(file) : false;
}

MetroSurfaceDescription MetroTexturesDatabase::GetSurfaceSetFromFile(const MetroFSPath& file, const bool allMips) const {
    if (this->Good()) {
        return mImpl->GetSurfaceSetFromFile(file, allMips);
    } else {
        static MetroSurfaceDescription emptyResult;
        return emptyResult;
    }
}

MetroSurfaceDescription MetroTexturesDatabase::GetSurfaceSetFromName(const HashString& textureName, const bool allMips) const {
    if (this->Good()) {
        return mImpl->GetSurfaceSetFromName(textureName, allMips);
    } else {
        static MetroSurfaceDescription emptyResult;
        return emptyResult;
    }
}

size_t MetroTexturesDatabase::GetNumTextures() const {
    return this->Good() ? mImpl->GetNumTextures() : 0;
}

const CharString& MetroTexturesDatabase::GetTextureNameByIdx(const size_t idx) const {
    return this->Good() ? mImpl->GetTextureNameByIdx(idx) : kEmptyString;
}

void MetroTexturesDatabase::FillCommonInfoByIdx(const size_t idx, MetroTextureInfoCommon& info) const {
    if (this->Good()) {
        mImpl->FillCommonInfoByIdx(idx, info);
    }
}

void MetroTexturesDatabase::SetCommonInfoByIdx(const size_t idx, const MetroTextureInfoCommon& info) {
    if (this->Good()) {
        mImpl->SetCommonInfoByIdx(idx, info);
    }
}

void MetroTexturesDatabase::RemoveTextureByIdx(const size_t idx) {
    if (this->Good()) {
        mImpl->RemoveTextureByIdx(idx);
    }
}

void MetroTexturesDatabase::AddTexture(const MetroTextureInfoCommon& info) {
    if (this->Good()) {
        mImpl->AddTexture(info);
    }
}
