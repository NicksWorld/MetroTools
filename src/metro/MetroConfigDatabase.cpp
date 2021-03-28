#include "MetroConfigDatabase.h"
#include "MetroConfigNames.h"

#include <fstream>

MetroConfigsDatabase::MetroConfigsDatabase() : mIsDirty(false){

}
MetroConfigsDatabase::~MetroConfigsDatabase() {

}

bool MetroConfigsDatabase::LoadFromData(MemStream& stream) {
    bool result = false;

    mStatsTotalDecryptedNames = 0;
    mStatsTotalEncryptedNames = 0;

    size_t idx = 0;
    while (!stream.Ended()) {
        const uint32_t nameCrc = stream.ReadTyped<uint32_t>();
        const uint32_t bodySize = stream.ReadTyped<uint32_t>();

        ConfigInfo ci;
        ci.idx = idx;
        ci.nameCRC = nameCrc;
        ci.nameStr = ConfigNamesDB::Get().FindByCRC32(nameCrc);
        ci.offset = stream.GetCursor();
        ci.length = bodySize;

        if (ci.nameStr.empty()) {
            mStatsTotalEncryptedNames++;
        } else {
            mStatsTotalDecryptedNames++;
        }

        mConfigsChunks.emplace_back(ci);

        stream.SkipBytes(bodySize);
        ++idx;
    }

    if (!mConfigsChunks.empty()) {
        mStream = stream.Clone();
        result = true;
    }

    return result;
}

void MetroConfigsDatabase::Shutdown() {
    mConfigsChunks.clear();
    mStream = MemStream();
    mStatsTotalDecryptedNames = 0;
    mStatsTotalEncryptedNames = 0;
}

const MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(const uint32_t nameCRC) const {
    auto it = std::find_if(mConfigsChunks.begin(), mConfigsChunks.end(), [nameCRC](const ConfigInfo& ci)->bool {
        return ci.nameCRC == nameCRC;
    });

    return (it == mConfigsChunks.end()) ? nullptr : &(*it);
}

const MetroConfigsDatabase::ConfigInfo* MetroConfigsDatabase::FindFile(const CharString& name) const {
    auto it = std::find_if(mConfigsChunks.begin(), mConfigsChunks.end(), [&name](const ConfigInfo& ci)->bool {
        return ci.nameStr == name;
    });

    return (it == mConfigsChunks.end()) ? nullptr : &(*it);
}

MemStream MetroConfigsDatabase::GetFileStream(const CharString& name) const {
    auto i = this->FindFile(name);
    return (i == nullptr) ? MemStream() : mStream.Substream(i->offset, i->length);
}

bool MetroConfigsDatabase::ReplaceFileByIdx(const size_t chunkIdx, const MemStream& stream) {
    bool result = false;

    if (stream.Good() && chunkIdx < mConfigsChunks.size()) {
        ConfigInfo& currentInfo = mConfigsChunks[chunkIdx];

        //#NOTE_SK: easy case - new data size is the same as old, simply copy over
        if (currentInfo.length == stream.Length()) {
            mStream.SetCursor(currentInfo.offset);
            uint8_t* nonConstData = const_cast<uint8_t*>(mStream.GetDataAtCursor());
            std::memcpy(nonConstData, stream.Data(), stream.Length());
        } else {
            //#NOTE_SK: more complicated case - the new file data is smaller/bigger than the old one
            //          so we have to create new stream and update chunks table
            const int sizeDelta = scast<int>(stream.Length() - currentInfo.length);

            const size_t newStreamSize = mStream.Length() + sizeDelta;
            uint8_t* newStreamMem = rcast<uint8_t*>(malloc(newStreamSize));

            const uint8_t* oldStreamData = mStream.Data();
            const uint8_t* oldStreamDataTail = oldStreamData + (currentInfo.offset + currentInfo.length);
            const size_t oldStreamTailSize = mStream.Length() - (currentInfo.offset + currentInfo.length);

            // update current file info
            currentInfo.length += sizeDelta;

            if (chunkIdx != 0) {    // if this isn't the first file - copy everything that comes before it
                std::memcpy(newStreamMem, oldStreamData, currentInfo.offset);
            }

            // now copy the new file data over
            std::memcpy(newStreamMem + currentInfo.offset, stream.Data(), stream.Length());
            // and fix body size in mem
            *rcast<uint32_t*>(newStreamMem + (currentInfo.offset - sizeof(uint32_t))) = scast<uint32_t>(currentInfo.length);

            if ((chunkIdx + 1) != mConfigsChunks.size()) {  // if this isn't the last file - copy the tail and update other files info
                std::memcpy(newStreamMem + currentInfo.offset + currentInfo.length, oldStreamDataTail, oldStreamTailSize);

                for (size_t i = chunkIdx + 1, numFiles = mConfigsChunks.size(); i < numFiles; ++i) {
                    ConfigInfo& ci = mConfigsChunks[i];
                    ci.offset += sizeDelta;
                }
            }

            // now destroy the old stream and construct the new one owning the new memory
            mStream = MemStream(newStreamMem, newStreamSize, true);
        }

        this->SetDirty(true);

        result = true;
    }

    return result;
}

size_t MetroConfigsDatabase::GetNumFiles() const {
    return mConfigsChunks.size();
}

const MetroConfigsDatabase::ConfigInfo& MetroConfigsDatabase::GetFileByIdx(const size_t chunkIdx) const {
    return mConfigsChunks[chunkIdx];
}

const MemStream& MetroConfigsDatabase::GetDataStream() const {
    return mStream;
}

void MetroConfigsDatabase::SetDirty(const bool dirty) {
    mIsDirty = dirty;
}

bool MetroConfigsDatabase::IsDirty() const {
    return mIsDirty;
}
