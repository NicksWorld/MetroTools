#include "MetroFonts.h"
#include "MetroBinArchive.h"
#include "reflection/MetroReflection.h"


void MetroFontDescriptor::CharInfo::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, x);
    METRO_SERIALIZE_MEMBER(s, y);
    METRO_SERIALIZE_MEMBER(s, width);
    METRO_SERIALIZE_MEMBER(s, height);
    METRO_SERIALIZE_MEMBER(s, xoffset);
    METRO_SERIALIZE_MEMBER(s, yoffset);
    METRO_SERIALIZE_MEMBER(s, xadvance);
}

void MetroFontDescriptor::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_MEMBER(s, lang);
    METRO_SERIALIZE_MEMBER(s, name);
    METRO_SERIALIZE_MEMBER(s, texture);
    METRO_SERIALIZE_MEMBER(s, inv_scale);
    METRO_SERIALIZE_MEMBER(s, line_height);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, tcmap);
}

const CharString& MetroFontDescriptor::GetName() const {
    return this->name;
}

const CharString& MetroFontDescriptor::GetTextureName() const {
    return this->texture;
}

const vec2& MetroFontDescriptor::GetInvScale() const {
    return this->inv_scale;
}

float MetroFontDescriptor::GetLineHeight() const {
    return this->line_height;
}

size_t MetroFontDescriptor::GetCharsCount() const {
    return this->tcmap.size();
}

const MetroFontDescriptor::CharInfo& MetroFontDescriptor::GetCharinfo(const size_t idx) const {
    return this->tcmap[idx];
}

MetroFontDescriptor::CharInfo& MetroFontDescriptor::GetCharinfo(const size_t idx) {
    return this->tcmap[idx];
}


MetroFontsDatabase::MetroFontsDatabase() {
}
MetroFontsDatabase::~MetroFontsDatabase() {
}

MetroFontsDatabase& MetroFontsDatabase::Get(const MetroLanguage lng) {
    static MetroFontsDatabase _singletons[scast<size_t>(MetroLanguage::Count)];
    return _singletons[scast<size_t>(lng)];
}

CharString MetroFontsDatabase::MakeFontDBPath(const MetroLanguage lng) {
    CharString binName = R"(content\scripts\fonts_)";
    binName += MetroLanguagesStr[scast<size_t>(lng)] + ".bin";
    return std::move(binName);
}

bool MetroFontsDatabase::LoadFromData(MemStream& stream) {
    MetroBinArchive bin("fonts_xx.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
    StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, descriptions);

    return !this->descriptions.empty();
}

void MetroFontsDatabase::SaveToData(MemWriteStream& stream) {
    MetroReflectionBinaryWriteStream tempStream(stream);
    MetroBinArchive bin(stream);
    tempStream.SetSTable(bin.GetSTable());

    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(tempStream, descriptions);

    bin.Finalize();
}

CharString MetroFontsDatabase::SaveToJsonString() {
    MetroReflectionJsonWriteStream stream;
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(stream, descriptions);
    return std::move(stream.WriteToString());
}

void MetroFontsDatabase::LoadFromJsonString(const CharString& jsonStr) {
    MetroReflectionJsonReadStream stream(jsonStr);
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(stream, descriptions);
}

void MetroFontsDatabase::Shutdown() {
    this->descriptions.clear();
}

size_t MetroFontsDatabase::GetFontsCount() const {
    return this->descriptions.size();
}

const MetroFontDescriptor& MetroFontsDatabase::GetFontDescriptor(const size_t idx) const {
    return this->descriptions[idx];
}

MetroFontDescriptor& MetroFontsDatabase::GetFontDescriptor(const size_t idx) {
    return this->descriptions[idx];
}
