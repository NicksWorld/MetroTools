#pragma once
#include "MetroTypes.h"

class MetroReflectionStream;

class MetroFontDescriptor {
public:
    struct CharInfo {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        uint16_t xoffset;
        uint16_t yoffset;
        uint16_t xadvance;

        void Serialize(MetroReflectionStream& s);
    };

    void Serialize(MetroReflectionStream& s);

    const CharString&   GetName() const;
    const CharString&   GetTextureName() const;
    const vec2&         GetInvScale() const;
    float               GetLineHeight() const;
    size_t              GetCharsCount() const;
    const CharInfo&     GetCharinfo(const size_t idx) const;
    CharInfo&           GetCharinfo(const size_t idx);

private:
    CharString          lang;
    CharString          name;
    CharString          texture;
    vec2                inv_scale;
    float               line_height;
    MyArray<CharInfo>   tcmap;
};

class MetroFontsDatabase {
protected:
    MetroFontsDatabase();
    ~MetroFontsDatabase();

public:
    MetroFontsDatabase(MetroFontsDatabase const&) = delete;
    void operator=(MetroFontsDatabase const&) = delete;

public:
    static MetroFontsDatabase&  Get(const MetroLanguage lng);
    static CharString           MakeFontDBPath(const MetroLanguage lng);

public:
    bool                        LoadFromData(MemStream& stream);
    void                        SaveToData(MemWriteStream& stream);
    CharString                  SaveToJsonString();
    void                        LoadFromJsonString(const CharString& jsonStr);
    void                        Shutdown();

    size_t                      GetFontsCount() const;
    const MetroFontDescriptor&  GetFontDescriptor(const size_t idx) const;
    MetroFontDescriptor&        GetFontDescriptor(const size_t idx);

private:
    MyArray<MetroFontDescriptor>    descriptions;
};
