#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "Surface.h"

class MetroModel;
class MetroModelBase;
class MetroLevel;

namespace u4a {

class Model;
class LevelGeo;

class ResourcesManager {
    ResourcesManager();
    ~ResourcesManager();

public:
    IMPL_SINGLETON(ResourcesManager)

public:
    bool        Initialize();
    void        Shutdown();

    void        Clear();

    void        SetLoadHighRes(const bool load);

    Surface     GetSurface(const HashString& name);
    Texture*    GetSimpleTexture(const HashString& name, const bool linear);
    Texture*    GetLMapTexture(const HashString& name);
    Texture*    GetHMapTexture(const HashString& name, const size_t w, const size_t h);
    Model*      GetModel(const HashString& name, const bool needAnimations);

    Model*      ConstructModel(MetroModelBase* model);
    LevelGeo*   ConstructLevelGeo(MetroLevel* level);

private:
    Surface     LoadSurface(const HashString& name);
    Texture*    LoadSimpleTexture(const HashString& name, const bool linear);
    Texture*    LoadLMapTexture(const HashString& name);
    Texture*    LoadHMapTexture(const HashString& name, const size_t w, const size_t h);
    Model*      LoadModel(const HashString& name, const bool needAnimations);

private:
    bool                            mLoadHighRes;

    Texture*                        mFallbackBase;
    Texture*                        mFallbackNormal;
    Texture*                        mFallbackBump;

    MyDict<HashString, Surface>     mSurfaces;
    MyDict<HashString, Texture*>    mTextures;
    MyDict<HashString, Model*>      mModels;
    MyDict<HashString, LevelGeo*>   mLevelGeos;

    MyArray<Model*>                 mDanglingModels;
    MyArray<LevelGeo*>              mDanglingLevelGeos;
};

} // namespace u4a
