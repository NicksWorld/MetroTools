#pragma once
#include "mycommon.h"

class MetroTypedStrings {
    IMPL_SINGLETON(MetroTypedStrings)

protected:
    MetroTypedStrings();
    ~MetroTypedStrings();

public:
    bool                Initialize(const fs::path& gameFolder);

    const TypedString&  GetString(const uint32_t crc32) const;

private:
    MyDict<uint32_t, TypedString>   mStrings;
};
