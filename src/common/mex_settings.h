#pragma once
#include "mycommon.h"
#include "pugixml.hpp"

class MEXSettings {
    IMPL_SINGLETON(MEXSettings)

protected:
    MEXSettings();
    ~MEXSettings();

public:
    void    SetFolder(const fs::path& folder);

    bool    Load();
    bool    Save();

    void    InitDefaults();

private:
    // extraction
    bool    LoadExtraction(pugi::xml_document& doc);
    bool    SaveExtraction(pugi::xml_document& doc);

    // open history
    bool    LoadOpenHistory(pugi::xml_document& doc);
    bool    SaveOpenHistory(pugi::xml_document& doc);

public:
    //// Extraction options
    struct Extraction {
        enum class MdlFormat : size_t {
            Obj,
            Fbx
        };
        enum class TexFormat : size_t {
            Dds,        // Dx10+
            LegacyDds,  // Dx9
            Tga,
            Png
        };
        enum class SndFormat : size_t {
            Ogg,
            Wav
        };

        // models
        MdlFormat   modelFormat;
        bool        modelSaveWithAnims;
        bool        modelAnimsSeparate;
        bool        modelSaveWithTextures;
        bool        modelExcludeCollision;
        bool        modelSaveSurfaceSet;
        bool        modelSaveLods;
        // textures
        TexFormat   textureFormat;
        // sounds
        SndFormat   soundFormat;
        // stuff
        bool        askEveryTime;
    } extraction;

    struct OpenHistory
    {
        WStringArray archives;
        WStringArray folders;
    } openHistory;

private:
    fs::path    mFolder;
};
