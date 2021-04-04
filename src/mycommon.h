#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <filesystem>
#include <memory>
#include <numeric>
#include <algorithm>
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
uint32_t Hash_CalculateCRC32(const uint8_t* data, const size_t dataLength);
uint32_t Hash_CalculateCRC32(const StringView& view);
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

    return std::move(result);
}

inline CharString StrWideToUtf8(const WideString&) {
    //CharString result = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(source);
    //return std::move(result);
    assert(false);
    return kEmptyString;
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

inline MyArray<StringView> StrSplitViews(const StringView& s, const char delimiter) {
    return __utils::StrSplitCommon<StringView>(s, delimiter);
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
        //#NOTE_SK: dirty hack to own pointer
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
    MemStream(MemStream&& other)
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

    inline MemStream& operator =(MemStream&& other) {
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
        return this->cursor == this->length;
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

        return std::move(result);
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

    MemStream GetChunkStream(const size_t chunkId) {
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
