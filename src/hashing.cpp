#include "mycommon.h"
#include "xxhash.h"

uint32_t Hash_CalculateXX(const uint8_t* data, const size_t dataLength) {
    return XXH32(data, dataLength, 0);
}

uint32_t Hash_CalculateXX(const StringView& view) {
    return view.empty() ? 0 : XXH32(view.data(), view.length(), 0);
}
