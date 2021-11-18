#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <filesystem>
#include <memory>
#include <numeric>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cuchar>


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define STRINGIFY_UTIL_(s) #s
#define STRINGIFY(s) STRINGIFY_UTIL_(s)

namespace fs = std::filesystem;

template <typename T>
using MyArray = std::vector<T>;
template <typename K, typename T>
using MyDict = std::unordered_map<K, T>;
template <typename T>
using MyDeque = std::deque<T>;
using CharString = std::string;
using StringView = std::string_view;
using WideString = std::wstring;
using WStringView = std::wstring_view;
using StringArray = MyArray<CharString>;
using WStringArray = MyArray<WideString>;
using BytesArray = MyArray<uint8_t>;

template <typename T>
using StrongPtr = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr StrongPtr<T> MakeStrongPtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using RefPtr = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr RefPtr<T> MakeRefPtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename U>
constexpr RefPtr<T> SCastRefPtr(const RefPtr<U>& p) noexcept {
    return std::static_pointer_cast<T>(p);
}

using MyHandle = size_t;

using flags8 = uint8_t;
using flags64 = uint64_t;

struct EntityLink {
    uint16_t value;
};

static const uint32_t   kInvalidValue32 = ~0u;
static const size_t     kInvalidValue = ~0;
static const MyHandle   kInvalidHandle = ~0;
static const CharString kEmptyString;
static const char       kPathSeparator = '\\';


#define rcast reinterpret_cast
#define scast static_cast

#ifdef __GNUC__
#define PACKED_STRUCT_BEGIN
#define PACKED_STRUCT_END __attribute__((__packed__))
#else
#define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
#define PACKED_STRUCT_END __pragma(pack(pop))
#endif

// hashing
constexpr uint32_t sCRC32Table[256] = {
    0x00000000u, 0x77073096u, 0xee0e612cu, 0x990951bau,
    0x076dc419u, 0x706af48fu, 0xe963a535u, 0x9e6495a3u,
    0x0edb8832u, 0x79dcb8a4u, 0xe0d5e91eu, 0x97d2d988u,
    0x09b64c2bu, 0x7eb17cbdu, 0xe7b82d07u, 0x90bf1d91u,
    0x1db71064u, 0x6ab020f2u, 0xf3b97148u, 0x84be41deu,
    0x1adad47du, 0x6ddde4ebu, 0xf4d4b551u, 0x83d385c7u,
    0x136c9856u, 0x646ba8c0u, 0xfd62f97au, 0x8a65c9ecu,
    0x14015c4fu, 0x63066cd9u, 0xfa0f3d63u, 0x8d080df5u,
    0x3b6e20c8u, 0x4c69105eu, 0xd56041e4u, 0xa2677172u,
    0x3c03e4d1u, 0x4b04d447u, 0xd20d85fdu, 0xa50ab56bu,
    0x35b5a8fau, 0x42b2986cu, 0xdbbbc9d6u, 0xacbcf940u,
    0x32d86ce3u, 0x45df5c75u, 0xdcd60dcfu, 0xabd13d59u,
    0x26d930acu, 0x51de003au, 0xc8d75180u, 0xbfd06116u,
    0x21b4f4b5u, 0x56b3c423u, 0xcfba9599u, 0xb8bda50fu,
    0x2802b89eu, 0x5f058808u, 0xc60cd9b2u, 0xb10be924u,
    0x2f6f7c87u, 0x58684c11u, 0xc1611dabu, 0xb6662d3du,
    0x76dc4190u, 0x01db7106u, 0x98d220bcu, 0xefd5102au,
    0x71b18589u, 0x06b6b51fu, 0x9fbfe4a5u, 0xe8b8d433u,
    0x7807c9a2u, 0x0f00f934u, 0x9609a88eu, 0xe10e9818u,
    0x7f6a0dbbu, 0x086d3d2du, 0x91646c97u, 0xe6635c01u,
    0x6b6b51f4u, 0x1c6c6162u, 0x856530d8u, 0xf262004eu,
    0x6c0695edu, 0x1b01a57bu, 0x8208f4c1u, 0xf50fc457u,
    0x65b0d9c6u, 0x12b7e950u, 0x8bbeb8eau, 0xfcb9887cu,
    0x62dd1ddfu, 0x15da2d49u, 0x8cd37cf3u, 0xfbd44c65u,
    0x4db26158u, 0x3ab551ceu, 0xa3bc0074u, 0xd4bb30e2u,
    0x4adfa541u, 0x3dd895d7u, 0xa4d1c46du, 0xd3d6f4fbu,
    0x4369e96au, 0x346ed9fcu, 0xad678846u, 0xda60b8d0u,
    0x44042d73u, 0x33031de5u, 0xaa0a4c5fu, 0xdd0d7cc9u,
    0x5005713cu, 0x270241aau, 0xbe0b1010u, 0xc90c2086u,
    0x5768b525u, 0x206f85b3u, 0xb966d409u, 0xce61e49fu,
    0x5edef90eu, 0x29d9c998u, 0xb0d09822u, 0xc7d7a8b4u,
    0x59b33d17u, 0x2eb40d81u, 0xb7bd5c3bu, 0xc0ba6cadu,
    0xedb88320u, 0x9abfb3b6u, 0x03b6e20cu, 0x74b1d29au,
    0xead54739u, 0x9dd277afu, 0x04db2615u, 0x73dc1683u,
    0xe3630b12u, 0x94643b84u, 0x0d6d6a3eu, 0x7a6a5aa8u,
    0xe40ecf0bu, 0x9309ff9du, 0x0a00ae27u, 0x7d079eb1u,
    0xf00f9344u, 0x8708a3d2u, 0x1e01f268u, 0x6906c2feu,
    0xf762575du, 0x806567cbu, 0x196c3671u, 0x6e6b06e7u,
    0xfed41b76u, 0x89d32be0u, 0x10da7a5au, 0x67dd4accu,
    0xf9b9df6fu, 0x8ebeeff9u, 0x17b7be43u, 0x60b08ed5u,
    0xd6d6a3e8u, 0xa1d1937eu, 0x38d8c2c4u, 0x4fdff252u,
    0xd1bb67f1u, 0xa6bc5767u, 0x3fb506ddu, 0x48b2364bu,
    0xd80d2bdau, 0xaf0a1b4cu, 0x36034af6u, 0x41047a60u,
    0xdf60efc3u, 0xa867df55u, 0x316e8eefu, 0x4669be79u,
    0xcb61b38cu, 0xbc66831au, 0x256fd2a0u, 0x5268e236u,
    0xcc0c7795u, 0xbb0b4703u, 0x220216b9u, 0x5505262fu,
    0xc5ba3bbeu, 0xb2bd0b28u, 0x2bb45a92u, 0x5cb36a04u,
    0xc2d7ffa7u, 0xb5d0cf31u, 0x2cd99e8bu, 0x5bdeae1du,
    0x9b64c2b0u, 0xec63f226u, 0x756aa39cu, 0x026d930au,
    0x9c0906a9u, 0xeb0e363fu, 0x72076785u, 0x05005713u,
    0x95bf4a82u, 0xe2b87a14u, 0x7bb12baeu, 0x0cb61b38u,
    0x92d28e9bu, 0xe5d5be0du, 0x7cdcefb7u, 0x0bdbdf21u,
    0x86d3d2d4u, 0xf1d4e242u, 0x68ddb3f8u, 0x1fda836eu,
    0x81be16cdu, 0xf6b9265bu, 0x6fb077e1u, 0x18b74777u,
    0x88085ae6u, 0xff0f6a70u, 0x66063bcau, 0x11010b5cu,
    0x8f659effu, 0xf862ae69u, 0x616bffd3u, 0x166ccf45u,
    0xa00ae278u, 0xd70dd2eeu, 0x4e048354u, 0x3903b3c2u,
    0xa7672661u, 0xd06016f7u, 0x4969474du, 0x3e6e77dbu,
    0xaed16a4au, 0xd9d65adcu, 0x40df0b66u, 0x37d83bf0u,
    0xa9bcae53u, 0xdebb9ec5u, 0x47b2cf7fu, 0x30b5ffe9u,
    0xbdbdf21cu, 0xcabac28au, 0x53b39330u, 0x24b4a3a6u,
    0xbad03605u, 0xcdd70693u, 0x54de5729u, 0x23d967bfu,
    0xb3667a2eu, 0xc4614ab8u, 0x5d681b02u, 0x2a6f2b94u,
    0xb40bbe37u, 0xc30c8ea1u, 0x5a05df1bu, 0x2d02ef8du
};

template<typename T>
constexpr uint32_t Hash_CalculateCRC32(const T* data, const size_t dataLength) {
    uint32_t result = ~0u;

    for (size_t i = 0; i < dataLength; ++i) {
        result = sCRC32Table[(result ^ data[i]) & 0xFF] ^ (result >> 8);
    }

    return (result ^ (~0u));
}
constexpr uint32_t Hash_CalculateCRC32(const StringView& view) {
    return view.empty() ? 0 : Hash_CalculateCRC32(view.data(), view.length());
}
constexpr uint32_t CRC32(const StringView& view) {
    return view.empty() ? 0 : Hash_CalculateCRC32(view.data(), view.length());
}

class Crc32Stream {
public:
    void Update(const void* data, const size_t dataLength) {
        const uint8_t* bytes = rcast<const uint8_t*>(data);

        for (size_t i = 0; i < dataLength; ++i) {
            hash = sCRC32Table[(hash ^ bytes[i]) & 0xFF] ^ (hash >> 8);
        }
    }

    uint32_t Finalize() const {
        return (hash ^ (~0u));
    }

private:
    uint32_t hash = ~0u;
};

uint32_t Hash_CalculateXX(const uint8_t* data, const size_t dataLength);
uint32_t Hash_CalculateXX(const StringView& view);

// encoding
CharString Encode_BytesToBase64(const uint8_t* data, const size_t dataLength);
BytesArray Encode_Base64ToBytes(const CharString& s);


struct TypedString {
    enum {
        TS_DEFAULT_STRING   = 0,
        TS_OBJECT_CLSID     = 1,
        TS_SCRIPT_CLSID     = 2,
        TS_LOCATOR_ID       = 3,
        TS_SDATA_KEY        = 4,
        TS_TEXTURE_SET      = 5,
    };

    TypedString() : type(TS_DEFAULT_STRING), crc32(0u) {}
    TypedString(const TypedString& other) : type(other.type), crc32(other.crc32), str(other.str) {}
    TypedString(const CharString& other, const uint32_t _type) { *this = other; this->type = _type; }

    inline void operator =(const TypedString& other) { type = other.type; crc32 = other.crc32; str = other.str; }
    inline void operator =(const CharString& other) { type = TS_DEFAULT_STRING; crc32 = Hash_CalculateCRC32(other); str = other; }

    inline bool operator ==(const TypedString& other) const { return crc32 == other.crc32; }
    inline bool operator !=(const TypedString& other) const { return crc32 != other.crc32; }

    inline bool operator <(const TypedString& other) const { return crc32 < other.crc32; }
    inline bool operator >(const TypedString& other) const { return crc32 > other.crc32; }

    inline bool Valid() const { return crc32 != 0u; }

    uint32_t    type;
    uint32_t    crc32;
    CharString  str;
};
static const TypedString kEmptyTypedString;

struct HashString {
    HashString() : hash(0u) {}
    HashString(const HashString& other) : hash(other.hash), str(other.str) {}
    HashString(const CharString& other) { *this = StringView(other); }
    HashString(const StringView& view) { *this = view; }

    inline void operator =(const HashString& other) { hash = other.hash; str = other.str; }
    inline void operator =(const CharString& other) { *this = StringView(other); }
    inline void operator =(const StringView& view) { str = view; hash = view.empty() ? 0u : Hash_CalculateXX(view); }

    inline bool operator ==(const HashString& other) const { return hash == other.hash; }
    inline bool operator !=(const HashString& other) const { return hash != other.hash; }

    inline bool operator <(const HashString& other) const { return hash < other.hash; }
    inline bool operator >(const HashString& other) const { return hash > other.hash; }

    inline bool Valid() const { return hash != 0u; }

    uint32_t    hash;
    CharString  str;
};
static const HashString kEmptyHashString;

namespace std {
    template <> struct hash<HashString> {
        size_t operator()(const HashString& s) const {
            return scast<size_t>(s.hash);
        }
    };
}

inline bool StrStartsWith(const CharString& str, const CharString& beginning) {
    return (str.rfind(beginning, 0) == 0);
}

inline bool WStrStartsWith(const WideString& str, const WideString& beginning) {
    return (str.rfind(beginning, 0) == 0);
}

inline bool StrEndsWith(const CharString& str, const CharString& ending) {
    return str.size() >= ending.size() && str.compare(str.size() - ending.size(), ending.size(), ending) == 0;
}

inline bool StrContains(const CharString& str, const CharString& value) {
    return str.size() >= value.size() && str.find(value) != CharString::npos;
}

inline WideString StrUtf8ToWide(const CharString& source) {
    //WideString result = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(source);
    //return std::move(result);

    WideString result;

    std::mbstate_t state = {};
    char16_t c16;
    const char* ptr = source.data(), *end = source.data() + source.size();

    while (size_t rc = std::mbrtoc16(&c16, ptr, end - ptr + 1, &state)) {
        if (rc == (size_t)-2 || rc == (size_t)-1) {
            break;
        } else {
            result.push_back(scast<wchar_t>(c16));
            ptr += rc;
        }
    }

    return result;
}

inline CharString StrWideToUtf8(const WideString& source) {
    CharString result;

    for (wchar_t wc : source) {
        if (wc < 0x80) {            // U+0000..U+007F
            result.push_back(scast<char>(wc));
        }
        else if (wc < 0x800) {    // U+0080..U+07FF
            result.push_back(scast<char>(0xC0 | (wc >> 6)));
            result.push_back(scast<char>(0x80 | (wc & 0x3F)));
        }
        else {                    // U+0800..U+FFFF
            result.push_back(scast<char>(0xE0 | (wc >> 12)));
            result.push_back(scast<char>(0x80 | ((wc >> 6) & 0x3F)));
            result.push_back(scast<char>(0x80 | (wc & 0x3F)));
        }
    }

    return result;
}

// for string delimiter
namespace __utils {
    template <typename T>
    inline MyArray<T> StrSplitCommon(const T& s, const typename T::value_type delimiter) {
        MyArray<T> result;

        if (!s.empty()) {
            size_t pos_start = 0, pos_end;
            T token;

            while ((pos_end = s.find(delimiter, pos_start)) != T::npos) {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + 1;
                result.emplace_back(token);
            }

            result.emplace_back(s.substr(pos_start));
        }

        return std::move(result);
    }
} // namespace __utils

inline StringArray StrSplit(const CharString& s, const char delimiter) {
    return __utils::StrSplitCommon<CharString>(s, delimiter);
}

inline WStringArray WStrSplit(const WideString& s, const wchar_t delimiter) {
    return __utils::StrSplitCommon<WideString>(s, delimiter);
}

inline MyArray<StringView> StrSplitViews(const StringView& s, const char delimiter) {
    return __utils::StrSplitCommon<StringView>(s, delimiter);
}

inline MyArray<WStringView> WStrSplitViews(const WStringView& s, const wchar_t delimiter) {
    return __utils::StrSplitCommon<WStringView>(s, delimiter);
}


struct StringsTable {
    const char* GetString(const size_t idx) const {
        return this->strings[idx];
    }

    uint32_t AddString(const HashString& str) {
        uint32_t result = kInvalidValue32;
        for (size_t i = 0, end = stringsAdded.size(); i < end; ++i) {
            const HashString& hs = this->stringsAdded[i];
            if (hs == str) {
                result = scast<uint32_t>(i);
                break;
            }
        }

        if (result == kInvalidValue32) {
            result = scast<uint32_t>(stringsAdded.size());
            stringsAdded.push_back(str);
        }

        return result;
    }

    MyArray<char>        data;
    MyArray<const char*> strings;       // holds strings when reading
    MyArray<HashString>  stringsAdded;  // holds strings when writing
};


class MemStream {
    using OwnedPtrType = std::shared_ptr<uint8_t>;

public:
    MemStream()
        : data(nullptr)
        , length(0)
        , cursor(0) {
    }

    MemStream(const void* _data, const size_t _size, const bool _ownMem = false)
        : data(rcast<const uint8_t*>(_data))
        , length(_size)
        , cursor(0)
    {
        //#NOTE_SK: dirty hack to own a pointer
        if (_ownMem) {
            ownedPtr = OwnedPtrType(const_cast<uint8_t*>(data), free);
        }
    }
    MemStream(const MemStream& other)
        : data(other.data)
        , length(other.length)
        , cursor(other.cursor)
        , ownedPtr(other.ownedPtr)
        , name(other.name) {
    }
    MemStream(MemStream&& other) noexcept
        : data(other.data)
        , length(other.length)
        , cursor(other.cursor)
        , ownedPtr(other.ownedPtr)
        , name(other.name) {
    }
    ~MemStream() {
    }

    inline MemStream& operator =(const MemStream& other) {
        this->data = other.data;
        this->length = other.length;
        this->cursor = other.cursor;
        this->ownedPtr = other.ownedPtr;
        this->name = other.name;
        return *this;
    }

    inline MemStream& operator =(MemStream&& other) noexcept {
        this->data = other.data;
        this->length = other.length;
        this->cursor = other.cursor;
        this->ownedPtr.swap(other.ownedPtr);
        this->name.swap(other.name);
        return *this;
    }

    inline void SetWindow(const size_t wndOffset, const size_t wndLength) {
        this->cursor = wndOffset;
        this->length = wndOffset + wndLength;
    }

    inline operator bool() const {
        return this->Good();
    }

    inline bool Good() const {
        return (this->data != nullptr && this->length > 0 && !this->Ended());
    }

    inline bool Ended() const {
        return this->cursor >= this->length;
    }

    inline size_t Length() const {
        return this->length;
    }

    inline size_t Remains() const {
        return this->length - this->cursor;
    }

    inline const uint8_t* Data() const {
        return this->data;
    }

    inline const CharString& Name() const {
        return this->name;
    }

    inline void SetName(const CharString& n) {
        this->name = n;
    }

    void ReadToBuffer(void* buffer, const size_t bytesToRead) {
        if (this->cursor + bytesToRead <= this->length) {
            std::memcpy(buffer, this->data + this->cursor, bytesToRead);
            this->cursor += bytesToRead;
        }
    }

    void SkipBytes(const size_t bytesToSkip) {
        if (this->cursor + bytesToSkip <= this->length) {
            this->cursor += bytesToSkip;
        } else {
            this->cursor = this->length;
        }
    }

    template <typename T>
    T ReadTyped() {
        T result = T(0);
        if (this->cursor + sizeof(T) <= this->length) {
            result = *rcast<const T*>(this->data + this->cursor);
            this->cursor += sizeof(T);
        }
        return result;
    }

    template <typename T>
    void ReadStruct(T& s) {
        this->ReadToBuffer(&s, sizeof(T));
    }

#define _IMPL_READ_FOR_TYPE(type, name) \
    inline type Read##name() {          \
        return this->ReadTyped<type>(); \
    }

    _IMPL_READ_FOR_TYPE(int8_t, I8);
    _IMPL_READ_FOR_TYPE(uint8_t, U8);
    _IMPL_READ_FOR_TYPE(int16_t, I16);
    _IMPL_READ_FOR_TYPE(uint16_t, U16);
    _IMPL_READ_FOR_TYPE(int32_t, I32);
    _IMPL_READ_FOR_TYPE(uint32_t, U32);
    _IMPL_READ_FOR_TYPE(int64_t, I64);
    _IMPL_READ_FOR_TYPE(uint64_t, U64);
    _IMPL_READ_FOR_TYPE(bool, Bool);
    _IMPL_READ_FOR_TYPE(float, F32);
    _IMPL_READ_FOR_TYPE(double, F64);

#undef _IMPL_READ_FOR_TYPE

    CharString ReadStringZ() {
        CharString result;

        size_t i = this->cursor;
        const char* ptr = rcast<const char*>(this->data);
        for (; i < this->length; ++i) {
            if (!ptr[i]) {
                break;
            }
        }

        result.assign(ptr + this->cursor, i - this->cursor);
        this->cursor = i + 1;

        return result;
    }

    inline size_t GetCursor() const {
        return this->cursor;
    }

    void SetCursor(const size_t pos) {
        this->cursor = std::min<size_t>(pos, this->length);
    }

    inline const uint8_t* GetDataAtCursor() const {
        return this->data + this->cursor;
    }

    MemStream Substream(const size_t subStreamLength) const {
        const size_t allowedLength = ((this->cursor + subStreamLength) > this->Length()) ? (this->Length() - this->cursor) : subStreamLength;
        return MemStream(this->GetDataAtCursor(), allowedLength);
    }

    MemStream Substream(const size_t subStreamOffset, const size_t subStreamLength) const {
        const size_t allowedOffset = (subStreamOffset > this->Length()) ? this->Length() : subStreamOffset;
        const size_t allowedLength = ((allowedOffset + subStreamLength) > this->Length()) ? (this->Length() - allowedOffset) : subStreamLength;
        return MemStream(this->data + allowedOffset, allowedLength);
    }

    MemStream Clone() const {
        if (this->ownedPtr) {
            return *this;
        } else {
            void* dataCopy = malloc(this->Length());
            assert(dataCopy != nullptr);
            memcpy(dataCopy, this->data, this->Length());
            return MemStream(dataCopy, this->Length(), true);
        }
    }

private:
    const uint8_t*  data;
    size_t          length;
    size_t          cursor;
    OwnedPtrType    ownedPtr;
    CharString      name;
};


class MemWriteStream {
public:
    MemWriteStream(const size_t startupSize = 4096) { mBuffer.reserve(startupSize); }
    ~MemWriteStream() {}

    void Swap(MemWriteStream& other) {
        mBuffer.swap(other.mBuffer);
    }

    void SwapBuffer(BytesArray& buffer) {
        mBuffer.swap(buffer);
    }

    void Write(const void* data, const size_t length) {
        const size_t cursor = this->GetWrittenBytesCount();
        mBuffer.resize(mBuffer.size() + length);
        memcpy(mBuffer.data() + cursor, data, length);
    }

    void WriteDupByte(const uint8_t value, const size_t numBytes) {
        const size_t cursor = this->GetWrittenBytesCount();
        mBuffer.resize(mBuffer.size() + numBytes);
        memset(mBuffer.data() + cursor, scast<int>(value), numBytes);
    }

    template <typename T>
    void Write(const T& v) {
        this->Write(&v, sizeof(v));
    }

#define _IMPL_WRITE_FOR_TYPE(type, name)        \
    inline void Write##name(const type& v) {    \
        this->Write<type>(v);                   \
    }

    _IMPL_WRITE_FOR_TYPE(int8_t, I8);
    _IMPL_WRITE_FOR_TYPE(uint8_t, U8);
    _IMPL_WRITE_FOR_TYPE(int16_t, I16);
    _IMPL_WRITE_FOR_TYPE(uint16_t, U16);
    _IMPL_WRITE_FOR_TYPE(int32_t, I32);
    _IMPL_WRITE_FOR_TYPE(uint32_t, U32);
    _IMPL_WRITE_FOR_TYPE(int64_t, I64);
    _IMPL_WRITE_FOR_TYPE(uint64_t, U64);
    _IMPL_WRITE_FOR_TYPE(bool, Bool);
    _IMPL_WRITE_FOR_TYPE(float, F32);
    _IMPL_WRITE_FOR_TYPE(double, F64);

#undef _IMPL_WRITE_FOR_TYPE

    void WriteStringZ(const CharString& str) {
        if (!str.empty()) {
            this->Write(str.c_str(), str.length());
        }
        this->WriteDupByte(0, 1);
    }

    inline void Append(const MemStream& stream) {
        this->Write(stream.Data(), stream.Length());
    }

    inline void Append(const MemWriteStream& stream) {
        this->Write(stream.Data(), stream.GetWrittenBytesCount());
    }

    size_t GetWrittenBytesCount() const {
        return mBuffer.size();
    }

    void* Data() {
        return mBuffer.data();
    }

    const void* Data() const {
        return mBuffer.data();
    }

    inline void SwapToBytesArray(BytesArray& dst) {
        mBuffer.swap(dst);
    }

private:
    BytesArray  mBuffer;
};


template <typename T>
constexpr T SetBit(const T& v, const T& bit) {
    return v | bit;
}
template <typename T>
constexpr T RemoveBit(const T& v, const T& bit) {
    return v & ~bit;
}
template <typename T>
constexpr bool TestBit(const T& v, const T& bit) {
    return 0 != (v & bit);
}

inline uint32_t CountBitsU32(uint32_t x) {
    x = (x & 0x55555555) + ((x >>  1) & 0x55555555);
    x = (x & 0x33333333) + ((x >>  2) & 0x33333333);
    x = (x & 0x0F0F0F0F) + ((x >>  4) & 0x0F0F0F0F);
    x = (x & 0x00FF00FF) + ((x >>  8) & 0x00FF00FF);
    x = (x & 0x0000FFFF) + ((x >> 16) & 0x0000FFFF);
    return x;
}

#ifdef _MSC_VER
inline uint16_t EndianSwapBytes(const uint16_t x) {
    return _byteswap_ushort(x);
}
#else
constexpr uint16_t EndianSwapBytes(const uint16_t x) {
    return ((x >> 8) & 0xFF) | ((x << 8) & 0xFF00);
}
#endif

inline int16_t EndianSwapBytes(const int16_t x) {
    const uint16_t u = EndianSwapBytes(*rcast<const uint16_t*>(&x));
    return *rcast<const int16_t*>(&u);
}

#ifdef _MSC_VER
inline uint32_t EndianSwapBytes(const uint32_t x) {
    return _byteswap_ulong(x);
}
#else
constexpr uint32_t EndianSwapBytes(const uint32_t x) {
    return (((x & 0xFF) << 24) | (((x & 0xFF00) >> 8) << 16) | (((x & 0xFF0000) >> 16) << 8) | ((x & 0xFF000000) >> 24));
}
#endif

inline int32_t EndianSwapBytes(const int32_t x) {
    const uint32_t u = EndianSwapBytes(*rcast<const uint32_t*>(&x));
    return *rcast<const int32_t*>(&u);
}

inline float EndianSwapBytes(const float f) {
    union {
        float f;
        char b[4];
    } src, dst;

    src.f = f;
    dst.b[3] = src.b[0];
    dst.b[2] = src.b[1];
    dst.b[1] = src.b[2];
    dst.b[0] = src.b[3];
    return dst.f;
}

inline uint64_t EndianSwapBytes(const uint64_t x) {
    return _byteswap_uint64(x);
}

inline int64_t EndianSwapBytes(const int64_t x) {
    const uint64_t u = EndianSwapBytes(*rcast<const uint64_t*>(&x));
    return *rcast<const int64_t*>(&u);
}

template <char a, char b, char c, char d>
constexpr uint32_t MakeFourcc() {
    const uint32_t result = scast<uint32_t>(a) | (scast<uint32_t>(b) << 8) | (scast<uint32_t>(c) << 16) | (scast<uint32_t>(d) << 24);
    return result;
}

template <typename T>
constexpr bool IsPowerOfTwo(const T& x) {
    return (x != 0) && ((x & (x - 1)) == 0);
}

PACKED_STRUCT_BEGIN
template <size_t N>
struct MyBitset {
    uint32_t dwords[N];

    inline size_t CountOnes() const {
        size_t result = 0;
        for (uint32_t x : this->dwords) {
            result += CountBitsU32(x);
        }
        return result;
    }

    inline bool IsPresent(const size_t idx) const {
        const size_t i = idx >> 5;
        assert(i <= N-1);
        const uint32_t mask = 1 << (idx & 0x1F);
        return (this->dwords[i] & mask) == mask;
    }

    inline void SetBit(const size_t idx, const bool value) {
        const size_t i = idx >> 5;
        assert(i <= N - 1);
        const uint32_t mask = 1 << (idx & 0x1F);
        if (value) {
            this->dwords[i] |= mask;
        } else {
            this->dwords[i] &= ~mask;
        }
    }

    inline void Clear() {
        std::memset(this->dwords, 0, sizeof(this->dwords));
    }
    inline void Fill() {
        std::memset(this->dwords, 0xFF, sizeof(this->dwords));
    }
} PACKED_STRUCT_END;

using Bitset128 = MyBitset<4>;
using Bitset256 = MyBitset<8>;

PACKED_STRUCT_BEGIN
struct Bool8 {
    union {
        uint8_t val8;
        struct {
            bool    b0 : 1;
            bool    b1 : 1;
            bool    b2 : 1;
            bool    b3 : 1;
            bool    b4 : 1;
            bool    b5 : 1;
            bool    b6 : 1;
            bool    b7 : 1;
        };
    };

    inline Bool8& operator =(const uint8_t _val) {
        this->val8 = _val;
        return *this;
    }
} PACKED_STRUCT_END;
static_assert(sizeof(Bool8) == sizeof(uint8_t));

//struct Range {
//    size_t  left, right;
//
//    inline bool Empty() const {
//        return this->right == this->left;
//    }
//    inline bool Contains(const size_t value) const {
//        return value >= this->left && value <= this->right;
//    }
//    inline bool Contains(const Range& other) const {
//        return this->Contains(other.left) && this->Contains(other.right);
//    }
//    inline bool IsNeighbour(const size_t value) const {
//        return (value + 1 == this->left) || (this->right + 1 == value);
//    }
//    inline bool Absorb(const size_t value) {
//        if (this->Contains(value)) {
//            return true;
//        } else if (this->IsNeighbour(value)) {
//            this->left = (std::min)(this->left, value);
//            this->right = (std::max)(this->right, value);
//            return true;
//        } else {
//            return false;
//        }
//    }
//};
//
//struct RangesList {
//    MyArray<Range> ranges;
//
//    void AddValue(const size_t value) {
//        for (Range& r : ranges) {
//            if (r.Absorb(value)) {
//                break;
//            }
//        }
//    }
//};


class StreamChunker {
    struct ChunkInfo {
        size_t  id;
        size_t  offset;
        size_t  length;
    };

public:
    StreamChunker(MemStream& stream)
        : mStream(stream) {
        const size_t savedCursor = stream.GetCursor();

        while (!stream.Ended()) {
            const size_t chunkId = stream.ReadU32();
            const size_t chunkSize = stream.ReadU32();
            const size_t chunkOffset = stream.GetCursor();

            mChunks.push_back({ chunkId, chunkOffset, chunkSize });

            stream.SkipBytes(chunkSize);
        }

        stream.SetCursor(savedCursor);
    }

    MemStream GetChunkStream(const size_t chunkId) const {
        auto it = std::find_if(mChunks.begin(), mChunks.end(), [chunkId](const ChunkInfo& ci)->bool {
            return ci.id == chunkId;
        });

        if (it != mChunks.end()) {
            const ChunkInfo& ci = *it;
            return mStream.Substream(ci.offset, ci.length);
        } else {
            return MemStream();
        }
    }

    size_t GetChunksCount() const {
        return mChunks.size();
    }

    size_t GetChunkIDByIdx(const size_t idx) const {
        return mChunks[idx].id;
    }

    size_t GetChunkLengthByIdx(const size_t idx) const {
        return mChunks[idx].length;
    }

    MemStream GetChunkStreamByIdx(const size_t idx) const {
        const ChunkInfo& ci = mChunks[idx];
        return mStream.Substream(ci.offset, ci.length);
    }

private:
    MemStream&          mStream;
    MyArray<ChunkInfo>  mChunks;
};

struct ChunkWriteHelper {
    ChunkWriteHelper() = delete;
    ChunkWriteHelper(const ChunkWriteHelper&) = delete;
    ChunkWriteHelper(ChunkWriteHelper&&) = delete;
    ChunkWriteHelper(MemWriteStream& stream, const size_t chunkId)
        : mStream(stream)
        , mChunkId(chunkId)
        , mChunkOffset(stream.GetWrittenBytesCount())
        , mChunkSize(0) {
        mStream.WriteU32(scast<uint32_t>(mChunkId));
        mStream.WriteU32(0);
    }
    ~ChunkWriteHelper() {
        mChunkSize = mStream.GetWrittenBytesCount() - mChunkOffset - 8;
        uint8_t* ptr = rcast<uint8_t*>(mStream.Data());
        *rcast<uint32_t*>(ptr + mChunkOffset + 4) = scast<uint32_t>(mChunkSize);
    }

    MemWriteStream& mStream;
    size_t          mChunkId;
    size_t          mChunkOffset;
    size_t          mChunkSize;
};


#ifndef MySafeRelease
#define MySafeRelease(ptr)  \
    if (ptr) {              \
        (ptr)->Release();   \
        (ptr) = nullptr;    \
    }
#endif

#ifndef MySafeDelete
#define MySafeDelete(ptr)   \
    if (ptr) {              \
        delete (ptr);       \
        (ptr) = nullptr;    \
    }
#endif


#define IMPL_SINGLETON(T)           \
public:                             \
T(T const&) = delete;               \
void operator=(T const&) = delete;  \
static T& Get() {                   \
    static T _instance;             \
    return _instance;               \
}


#define INHERITED_CLASS(superName)  \
using Base = superName;


// File I/O
MemStream           OSReadFile(const fs::path& filePath);
MemStream           OSReadFileEX(const fs::path& filePath, const size_t subOffset, const size_t subLength);
size_t              OSWriteFile(const fs::path& filePath, const void* data, const size_t dataLength);
size_t              OSGetFileSize(const fs::path& filePath);
bool                OSPathExists(const fs::path& pathToCheck);
bool                OSPathIsFile(const fs::path& pathToCheck);
bool                OSPathIsFolder(const fs::path& pathToCheck);
MyArray<fs::path>   OSPathGetEntriesList(const fs::path& pathToCheck, const bool recursive, const bool onlyFiles, const fs::path& ext = fs::path());

#include "log.h"
