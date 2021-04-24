#include "mycommon.h"

static const char sBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

CharString Encode_BytesToBase64(const uint8_t* data, const size_t dataLength) {
    CharString result;
    result.reserve(4 * (dataLength + 3) / 3 + 2);

    int val = 0, valb = -6;
    for (size_t i = 0; i < dataLength; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result.push_back(sBase64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        result.push_back(sBase64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (result.size() % 4) {
        result.push_back('=');
    }

    return std::move(result);
}

BytesArray Encode_Base64ToBytes(const CharString& s) {
    BytesArray result;

    MyArray<int> T(256, -1);
    for (int i = 0; i < 64; ++i) {
        T[sBase64Chars[i]] = i;
    }

    int val = 0, valb = -8;
    for (char c : s) {
        if (T[c] == -1) {
            break;
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(scast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return std::move(result);
}
