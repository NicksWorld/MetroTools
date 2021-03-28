#include "MetroTypedStrings.h"
#include <fstream>

MetroTypedStrings::MetroTypedStrings() {
}
MetroTypedStrings::~MetroTypedStrings() {
}

bool MetroTypedStrings::Initialize(const fs::path& gameFolder) {
    bool result = false;

    mStrings.clear();

    fs::path filePath = gameFolder / "typed_strings.bin";
    std::ifstream file(filePath, std::ios::binary);
    if (file.good()) {
        BytesArray fileData;

        file.seekg(0, std::ios::end);
        fileData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData.data()), fileData.size());
        file.close();

        MemStream stream(fileData.data(), fileData.size());

        const uint32_t something = stream.ReadTyped<uint32_t>();
        const size_t numStrings = stream.ReadTyped<uint32_t>();

        mStrings.reserve(numStrings);
        for (size_t i = 0; i < numStrings; ++i) {
            const uint32_t tsType = stream.ReadTyped<uint16_t>();
            const CharString tsStr = stream.ReadStringZ();

            TypedString ts(tsStr, tsType);
            mStrings.insert({ ts.crc32, ts });
        }

        result = !mStrings.empty();
    }

    return result;
}

const TypedString& MetroTypedStrings::GetString(const uint32_t crc32) const {
    auto it = mStrings.find(crc32);
    if (it != mStrings.end()) {
        return it->second;
    } else {
        return kEmptyTypedString;
    }
}
