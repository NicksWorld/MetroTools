#include "ResourcesManager.h"

#include "metro/MetroContext.h"
#include "metro/MetroTexture.h"
#include "metro/MetroModel.h"
#include "metro/MetroLevel.h"

#include "engine/Model.h"
#include "engine/LevelGeo.h"

#include "log.h"

namespace u4a {

ResourcesManager::ResourcesManager()
    : mLoadHighRes(false)
    , mFallbackBase(nullptr)
    , mFallbackNormal(nullptr)
    , mFallbackBump(nullptr)
{
}
ResourcesManager::~ResourcesManager() {
}

bool ResourcesManager::Initialize() {
    bool result = false;

    // Base - pure white, opaque
    const uint32_t pureWhite = ~0u;
    mFallbackBase = new Texture();
    if (!mFallbackBase->CreateImmutable2D(Texture::Format::RGBA8_UNORM, 1, 1, 1, &pureWhite, Texture::Flag_SRGB)) {
        MySafeDelete(mFallbackBase);
    }

    // Normal - R-128, G-128, B-255, A-255
    const uint32_t pureNormal = 0xffff8080;
    mFallbackNormal = new Texture();
    if (!mFallbackNormal->CreateImmutable2D(Texture::Format::RGBA8_UNORM, 1, 1, 1, &pureNormal, 0)) {
        MySafeDelete(mFallbackNormal);
    }

    // Bump - R-255, G-235, B-57, A-152
    const uint32_t pureBump = 0x98ffeb39;
    mFallbackBump = new Texture();
    if (!mFallbackBump->CreateImmutable2D(Texture::Format::RGBA8_UNORM, 1, 1, 1, &pureBump, 0)) {
        MySafeDelete(mFallbackBump);
    }

    result = (mFallbackBase && mFallbackNormal && mFallbackBump);

    return result;
}

void ResourcesManager::Shutdown() {
    this->Clear();

    MySafeDelete(mFallbackBase);
    MySafeDelete(mFallbackNormal);
    MySafeDelete(mFallbackBump);
}

void ResourcesManager::Clear() {
    for (auto& p : mSurfaces) {
        if (p.second.base != mFallbackBase) {
            MySafeDelete(p.second.base);
        }
        if (p.second.normal != mFallbackNormal) {
            MySafeDelete(p.second.normal);
        }
        if (p.second.bump != mFallbackBump) {
            MySafeDelete(p.second.bump);
        }
    }
    mSurfaces.clear();

    for (auto& lm : mTextures) {
        MySafeDelete(lm.second);
    }
    mTextures.clear();

    for (auto& p : mModels) {
        MySafeDelete(p.second);
    }
    mModels.clear();

    for (auto& p : mLevelGeos) {
        MySafeDelete(p.second);
    }
    mLevelGeos.clear();

    for (auto p : mDanglingModels) {
        MySafeDelete(p);
    }
    mDanglingModels.clear();

    for (auto p : mDanglingLevelGeos) {
        MySafeDelete(p);
    }
    mDanglingLevelGeos.clear();
}

void ResourcesManager::SetLoadHighRes(const bool load) {
    mLoadHighRes = load;
}


Surface ResourcesManager::GetSurface(const HashString& name) {
    auto it = mSurfaces.find(name);
    if (it != mSurfaces.end()) {
        return it->second;
    } else {
        return this->LoadSurface(name);
    }
}

Texture* ResourcesManager::GetSimpleTexture(const HashString& name, const bool linear) {
    if (!name.Valid()) {
        return mFallbackBase;
    }

    auto it = mTextures.find(name);
    if (it != mTextures.end()) {
        return it->second;
    } else {
        return this->LoadSimpleTexture(name, linear);
    }
}

Texture* ResourcesManager::GetLMapTexture(const HashString& name) {
    auto it = mTextures.find(name);
    if (it != mTextures.end()) {
        return it->second;
    } else {
        return this->LoadLMapTexture(name);
    }
}

Texture* ResourcesManager::GetHMapTexture(const HashString& name, const size_t w, const size_t h) {
    auto it = mTextures.find(name);
    if (it != mTextures.end()) {
        return it->second;
    } else {
        return this->LoadHMapTexture(name, w, h);
    }
}

Model* ResourcesManager::GetModel(const HashString& name, const bool needAnimations) {
    auto it = mModels.find(name);
    if (it != mModels.end()) {
        return it->second;
    } else {
        return this->LoadModel(name, needAnimations);
    }
}

Model* ResourcesManager::ConstructModel(MetroModelBase* model) {
    Model* result = new Model();
    if (!result->Create(model)) {
        MySafeDelete(result);
    }

    if (result) {
        mDanglingModels.push_back(result);
    }

    return result;
}

LevelGeo* ResourcesManager::ConstructLevelGeo(MetroLevel* level) {
    LevelGeo* result = new LevelGeo();
    if (!result->Create(level)) {
        MySafeDelete(result);
    }

    if (result) {
        mDanglingLevelGeos.push_back(result);
    }

    return result;
}


static Texture* Util_LoadTexture(const StringArray& textureLevels, const size_t flags, const bool onlyBaseLevel) {
    Texture* result = nullptr;

    if (!textureLevels.empty()) {
        MyArray<MetroTexture*> loadedLevels;
        size_t numMips = 0;
        if (onlyBaseLevel) {
            const CharString& texName = textureLevels.back();
            MetroTexture* texture = new MetroTexture();
            if (texture->LoadFromPath(texName)) {
                loadedLevels.push_back(texture);
                numMips += texture->GetNumMips();
            } else {
                delete texture;
            }
        } else {
            for (const CharString& texName : textureLevels) {
                MetroTexture* texture = new MetroTexture();
                if (texture->LoadFromPath(texName)) {
                    loadedLevels.push_back(texture);
                    numMips += texture->GetNumMips();
                } else {
                    delete texture;
                }
            }
        }

        if (!loadedLevels.empty()) {
            result = new Texture();

            const MetroTexture* topLevel = loadedLevels.front();
            const size_t width = topLevel->GetWidth();
            const size_t height = topLevel->GetHeight();
            const bool isCubemap = topLevel->IsCubemap();
            const Texture::Format format = scast<Texture::Format>(topLevel->GetFormat());

            if (loadedLevels.size() == 1) {
                const bool success = isCubemap ?
                    result->CreateImmutableCube(format, width, height, numMips, topLevel->GetRawData(), flags) :
                    result->CreateImmutable2D(format, width, height, numMips, topLevel->GetRawData(), flags);

                if (!success) {
                    MySafeDelete(result);
                }
            } else {
                result->CreateStreamable2D(format, width, height, numMips, flags);
                size_t startMip = 0;
                for (const MetroTexture* level : loadedLevels) {
                    result->UploadMips(startMip, level->GetNumMips(), 0, level->GetRawData());
                    ++startMip;
                }

                if (!result->FinishStreamable2D()) {
                    MySafeDelete(result);
                }
            }

            for (MetroTexture* ptr : loadedLevels) {
                MySafeDelete(ptr);
            }
        }
    }

    return result;
}


Surface ResourcesManager::LoadSurface(const HashString& name) {
    Surface result;

    MetroSurfaceDescription desc = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromName(name, true);

    result.base = Util_LoadTexture(desc.albedoPaths, Texture::Flag_SRGB, !mLoadHighRes);
    result.normal = Util_LoadTexture(desc.normalmapPaths, 0, !mLoadHighRes);
    result.bump = Util_LoadTexture(desc.bumpPaths, 0, !mLoadHighRes);

    if (!result.base) {
        result.base = mFallbackBase;
        LogPrintF(LogLevel::Warning, "Failed to load texture %s", name.str.c_str());
    }
    if (!result.normal) {
        result.normal = mFallbackNormal;
    }
    if (!result.bump) {
        result.bump = mFallbackBump;
    }

    mSurfaces.insert({ name, result });

    return result;
}

Texture* ResourcesManager::LoadSimpleTexture(const HashString& name, const bool linear) {
    Texture* result = nullptr;

    StringArray textureLevels = MetroContext::Get().GetTexturesDB().GetAllLevels(name);
    result = Util_LoadTexture(textureLevels, linear ? 0 : Texture::Flag_SRGB, false);
    if (!result) {
        return mFallbackBase;
    }

    mTextures.insert({ name, result });

    return result;
}

Texture* ResourcesManager::LoadLMapTexture(const HashString& name) {
    Texture* result = nullptr;

    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileFromPath(name.str);
    if (stream.Good()) {
        const size_t version = stream.ReadTyped<uint32_t>();
        const size_t width = stream.ReadTyped<uint32_t>();
        const size_t height = stream.ReadTyped<uint32_t>();
        const Texture::Format format = Texture::Format::R8_UNORM;

        result = new Texture();
        if (!result->CreateImmutable2D(format, width, height, 1, stream.GetDataAtCursor(), Texture::Flag_GenMips)) {
            MySafeDelete(result);
        }
    }

    if (!result) {
        return mFallbackBase;
    }

    mTextures.insert({ name, result });

    return result;
}

Texture* ResourcesManager::LoadHMapTexture(const HashString& name, const size_t w, const size_t h) {
    Texture* result = nullptr;

    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileFromPath(name.str);
    if (stream.Good()) {
        const Texture::Format format = Texture::Format::R16_UNORM;

        result = new Texture();
        if (!result->CreateImmutable2D(format, w, h, 1, stream.GetDataAtCursor(), 0)) {
            MySafeDelete(result);
        }
    }

    if (!result) {
        return mFallbackBase;
    }

    mTextures.insert({ name, result });

    return result;
}

Model* ResourcesManager::LoadModel(const HashString& name, const bool needAnimations) {
    Model* result = nullptr;

#if 0
    MetroModel srcModel;
    if (srcModel.LoadFromName(name.str, needAnimations)) {
        result = new Model();
        if (!result->Create(&srcModel)) {
            MySafeDelete(result);
        }
    }
#else
    const uint32_t loadFlags = MetroModelLoadParams::LoadGeometry | MetroModelLoadParams::LoadSkeleton | MetroModelLoadParams::LoadTPresets;
    RefPtr<MetroModelBase> srcModel = MetroModelFactory::CreateModelFromFullName(name.str, loadFlags);
    if (srcModel) {
        result = new Model();
        if (!result->Create(srcModel.get())) {
            MySafeDelete(result);
        }
    }
#endif

    if (result) {
        mModels.insert({ name, result });
    }

    return result;
}

} // namespace u4a
