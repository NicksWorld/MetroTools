#include "MetroSessions.h"
#include "metro/reflection/MetroReflection.h"


MetroSession::MetroSession()
    : mGameVersion(MetroGameVersion::Unknown) {
}
MetroSession::~MetroSession() {
}

void MetroSession::Serialize(MetroReflectionStream& s) {
    uint32_t version = scast<uint32_t>(mGameVersion);
    CharString folder = mContentFolder.u8string();

    METRO_SERIALIZE_MEMBER(s, version);
    METRO_SERIALIZE_MEMBER(s, folder);

    mGameVersion = scast<MetroGameVersion>(version);
    mContentFolder = fs::u8path(folder);
}

void MetroSession::SetGameVersion(const MetroGameVersion version) {
    mGameVersion = version;
}

void MetroSession::SetContentFolder(const fs::path& folder) {
    mContentFolder = folder;
}

MetroGameVersion MetroSession::GetGameVersion() const {
    return mGameVersion;
}

const fs::path& MetroSession::GetContentFolder() const {
    return mContentFolder;
}



MetroSessionsList::MetroSessionsList() {
}
MetroSessionsList::~MetroSessionsList() {
}

bool MetroSessionsList::LoadFromFile(const fs::path& filePath) {
    bool result = false;

    MemStream fileData = OSReadFile(filePath);
    if (fileData) {
        CharString jsonStr = rcast<const char*>(fileData.Data());
        MetroReflectionJsonReadStream jsonStream(jsonStr);
        this->Serialize(jsonStream);
        result = true;
    }

    return result;
}

bool MetroSessionsList::SaveToFile(const fs::path& filePath) {
    bool result = false;

    MetroReflectionJsonWriteStream jsonStream;
    this->Serialize(jsonStream);
    CharString jsonStr = jsonStream.WriteToString();
    result = (!jsonStr.empty() && OSWriteFile(filePath, jsonStr.c_str(), jsonStr.length()));

    return result;
}

void MetroSessionsList::Serialize(MetroReflectionStream& s) {
    METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, sessions);
}

void MetroSessionsList::AddSession(const MetroSession& session) {
    this->sessions.push_back(session);
}

size_t MetroSessionsList::GetNumSessions() const {
    return this->sessions.size();
}

const MetroSession& MetroSessionsList::GetSession(const size_t idx) const {
    return this->sessions[idx];
}
