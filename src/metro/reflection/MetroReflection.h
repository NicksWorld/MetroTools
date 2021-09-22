#pragma once
#include "mycommon.h"
#include "mymath.h"

struct MetroReflectionFlags {
    static const uint8_t None           = 0;
    static const uint8_t HasDebugInfo   = 1;
    static const uint8_t Editor         = 2;
    static const uint8_t StringsTable   = 4;
    static const uint8_t Plain          = 8;
    static const uint8_t NoSections     = 16;
    static const uint8_t Multipart      = 32;

    // clearing
    static const uint8_t RemoveMultiChunkMask = ~Multipart;

    // defaults
    static const uint8_t DefaultOutFlags = StringsTable;
};


#define METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias)  sMetroRegisteredType##type##Alias##alias##Str
#define METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias)  sMetroRegisteredTypeArray##type##Alias##alias##Str

template <typename T>
const CharString& MetroTypeGetAlias() {
    static const CharString empty;
    assert(false);
    return empty;
}

template <typename T>
const CharString& MetroTypeArrayGetAlias() {
    static const CharString empty;
    assert(false);
    return empty;
}


#define METRO_REGISTER_TYPE_ALIAS(type, alias)                                                  \
template <> inline const CharString& MetroTypeGetAlias<type>() {                                \
    static const CharString METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias);  \
    return METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias);                                      \
}


#define METRO_REGISTER_TYPE_ARRAY_ALIAS(type, alias)                                                            \
template <> inline const CharString& MetroTypeArrayGetAlias<type>() {                                           \
    static const CharString METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias) "_array";   \
    return METRO_MAKE_TYPE_ARRAY_ALIAS_STRING_NAME(type, alias);                                                \
}


#define METRO_REGISTER_INHERITED_TYPE_ALIAS(type, baseType, alias)                                                      \
template <> inline const CharString& MetroTypeGetAlias<type>() {                                                        \
    static const CharString METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias) = STRINGIFY(alias) ", " STRINGIFY(baseType); \
    return METRO_MAKE_TYPE_ALIAS_STRING_NAME(type, alias);                                                              \
}



METRO_REGISTER_TYPE_ALIAS(bool, bool)
METRO_REGISTER_TYPE_ALIAS(uint8_t, u8)
METRO_REGISTER_TYPE_ALIAS(uint16_t, u16)
METRO_REGISTER_TYPE_ALIAS(uint32_t, u32)
METRO_REGISTER_TYPE_ALIAS(uint64_t, u64)
METRO_REGISTER_TYPE_ALIAS(int8_t, s8)
METRO_REGISTER_TYPE_ALIAS(int16_t, s16)
METRO_REGISTER_TYPE_ALIAS(int32_t, s32)
METRO_REGISTER_TYPE_ALIAS(int64_t, s64)
METRO_REGISTER_TYPE_ALIAS(float, fp32)
METRO_REGISTER_TYPE_ALIAS(fp32_q8, fp32_q8)
METRO_REGISTER_TYPE_ALIAS(vec2, vec2f)
METRO_REGISTER_TYPE_ALIAS(vec3, vec3f)
METRO_REGISTER_TYPE_ALIAS(vec4, vec4f)
METRO_REGISTER_TYPE_ALIAS(quat, vec4f)
METRO_REGISTER_TYPE_ALIAS(CharString, stringz)
METRO_REGISTER_TYPE_ALIAS(Bool8, bool8)
METRO_REGISTER_TYPE_ALIAS(flags32, flags32)
METRO_REGISTER_TYPE_ALIAS(ivec4, vec4i);
METRO_REGISTER_TYPE_ALIAS(vec4s16, vec4s16);
METRO_REGISTER_TYPE_ALIAS(ang3f, ang3f);

METRO_REGISTER_INHERITED_TYPE_ALIAS(color4f, vec4f, color)
METRO_REGISTER_INHERITED_TYPE_ALIAS(color32u, u32, color)
METRO_REGISTER_INHERITED_TYPE_ALIAS(pose_43T, matrix_43T, pose) // why in the fuck ???
METRO_REGISTER_INHERITED_TYPE_ALIAS(pose_43, matrix, pose)
METRO_REGISTER_INHERITED_TYPE_ALIAS(anglef, fp32, angle);
METRO_REGISTER_INHERITED_TYPE_ALIAS(EntityLink, uobject_link, entity_link)

METRO_REGISTER_TYPE_ARRAY_ALIAS(bool, bool)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint8_t, u8)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint16_t, u16)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint32_t, u32)
METRO_REGISTER_TYPE_ARRAY_ALIAS(uint64_t, u64)
METRO_REGISTER_TYPE_ARRAY_ALIAS(float, fp32)


// Base class for all reflection streams
class MetroReflectionStream {
public:
    enum Mode : size_t {
        IN,     // reading streams
        OUT     // writing streams
    };

public:
    MetroReflectionStream();
    virtual ~MetroReflectionStream();

    // Mode query
    inline Mode GetMode() const {
        return mMode;
    }

    inline bool IsIn() const {
        return mMode == Mode::IN;
    }

    inline bool IsOut() const {
        return mMode == Mode::OUT;
    }

    // Flags query
    inline uint8_t GetFlags() const {
        return mFlags;
    }

    inline bool HasDebugInfo() const {
        return TestBit(mFlags, MetroReflectionFlags::HasDebugInfo);
    }

    inline bool HasStringsTable() const {
        return TestBit(mFlags, MetroReflectionFlags::StringsTable);
    }

    inline bool HasNoSections() const {
        return TestBit(mFlags, MetroReflectionFlags::NoSections);
    }

    // State query
    virtual bool Good() const { return true; }

    inline void SetSectionName(const CharString& sectionName) {
        mSectionName = sectionName;
    }

    inline const CharString& GetSectionName() const {
        return mSectionName;
    }

    inline void SetUserData(const size_t userData) {
        mUserData = userData;
    }

    inline size_t GetUserData() const {
        return mUserData;
    }

    void SetSTable(StringsTable* stable) {
        mSTable = stable;
    }

    // Binary streams will need this
    virtual size_t GetCursor() const { return 0; }
    virtual size_t GetRemains() const { return 0; }
    virtual void SkipBytes(const size_t /*numBytes*/) { }

    virtual bool HasSomeMore() const = 0;

    // Serialization
    virtual void SerializeRawBytes(void* ptr, const size_t length) = 0;
    virtual void SerializeBool(bool& v);
    virtual void SerializeI8(int8_t& v);
    virtual void SerializeU8(uint8_t& v);
    virtual void SerializeI16(int16_t& v);
    virtual void SerializeU16(uint16_t& v);
    virtual void SerializeI32(int32_t& v);
    virtual void SerializeU32(uint32_t& v);
    virtual void SerializeI64(int64_t& v);
    virtual void SerializeU64(uint64_t& v);
    virtual void SerializeF32(float& v);
    virtual void SerializeF64(double& v);
    virtual void SerializeString(CharString& v);
    // Advanced
    virtual void SerializeFloats(float* fv, const size_t numFloats);
    virtual void SerializeInts(int* iv, const size_t numInts);
    virtual void FlushSectionTail(BytesArray& data) = 0;

    // Generic serialization of complex types
    template <typename T>
    inline void operator >>(T& v) {
        v.Serialize(*this);
    }

    // Metro data serialization details
    virtual bool SerializeEditorTag(const CharString& propName, const size_t chooseType = 0);
    virtual bool SerializeTypeInfo(const CharString& propName, const CharString& typeAlias);
    virtual MetroReflectionStream* OpenSection(const CharString& sectionName, const bool nameUnknown = false) = 0;
    virtual void CloseSection(MetroReflectionStream* section) = 0;

    int SkipSection(const CharString& sectionName, const bool nameUnknown = false) {
        MetroReflectionStream* section = this->OpenSection(sectionName, nameUnknown);;
        if (section) {
            int result = section->GetRemains();
            this->CloseSection(section);
            return result;
        } else {
            return -1;
        }
    }

    //NOTE_SK: ugly hack to support serializators like JSON
    virtual void BeginArray16(uint16_t& size);
    virtual void BeginArray32(uint32_t& size);
    virtual void EndArray() {}

    template <typename TSize>
    void BeginArray(TSize&) {}
    template <> void BeginArray<uint16_t>(uint16_t& size) {
        this->BeginArray16(size);
    }
    template <> void BeginArray<uint32_t>(uint32_t& size) {
        this->BeginArray32(size);
    }

    template <typename TElement, typename TSize>
    void SerializeArray(MyArray<TElement>& v) {
        TSize numElements = scast<TSize>(v.size());
        this->BeginArray(numElements);

        if (this->IsIn()) {
            v.resize(numElements);
        }

        for (TElement& e : v) {
            (*this) >> e;
        }

        this->EndArray();
    }

    template <typename T>
    bool SerializeStruct(const CharString& memberName, T& v) {
        MetroReflectionStream* s = this->OpenSection(memberName);
        if (s) {
            (*s) >> v;
            this->CloseSection(s);
        }

        return s != nullptr;
    }

    template <typename T>
    void SerializeStructArray(const CharString& memberName, MyArray<T>& v) {
        this->SerializeTypeInfo(memberName, "array");

        MetroReflectionStream* s = this->OpenSection(memberName);
        if (s) {
            s->SerializeTypeInfo("count", MetroTypeGetAlias<uint32_t>());

            uint32_t arraySize = scast<uint32_t>(v.size());
            (*s) >> arraySize;

            if (arraySize > 0) {
                if (s->IsIn()) {
                    if (arraySize != kInvalidValue32) {
                        v.reserve(arraySize);
                    }
                }

                int idx = 0;
                for (size_t i = 0; i < arraySize; ++i) {
                    if (!s->Good()) {   // for arrays of unknown size (like "entities" in level.bin in Redux)
                        break;
                    }

                    MetroReflectionStream* subS = nullptr;
                    if (s->IsIn()) {
                        v.resize(v.size() + 1);
                        subS = s->OpenSection(kEmptyString, true);
                    } else {
                        static char _subs_name[16] = { 0 };
                        sprintf_s(_subs_name, "rec_%04d", idx);
                        subS = s->OpenSection(_subs_name, true);
                        ++idx;
                    }

                    if (subS) {
                        (*subS) >> v[i];
                        s->CloseSection(subS);
                    }
                }
            }

            this->CloseSection(s);
        }
    }

protected:
    virtual void ReadStringZ(CharString& s) = 0;
    virtual void WriteStringZ(CharString& s) = 0;

protected:
    Mode            mMode;
    size_t          mUserData;
    CharString      mSectionName;
    StringsTable*   mSTable;
    uint8_t         mFlags;
};

// Base types serialization support
#define IMPLEMENT_BASE_TYPE_SERIALIZE(type, suffix)             \
inline void operator >>(MetroReflectionStream& s, type& v) {    \
    s.Serialize##suffix (v);                                    \
}

IMPLEMENT_BASE_TYPE_SERIALIZE(bool, Bool)
IMPLEMENT_BASE_TYPE_SERIALIZE(int8_t, I8)
IMPLEMENT_BASE_TYPE_SERIALIZE(uint8_t, U8)
IMPLEMENT_BASE_TYPE_SERIALIZE(int16_t, I16)
IMPLEMENT_BASE_TYPE_SERIALIZE(uint16_t, U16)
IMPLEMENT_BASE_TYPE_SERIALIZE(int32_t, I32)
IMPLEMENT_BASE_TYPE_SERIALIZE(uint32_t, U32)
IMPLEMENT_BASE_TYPE_SERIALIZE(int64_t, I64)
IMPLEMENT_BASE_TYPE_SERIALIZE(uint64_t, U64)
IMPLEMENT_BASE_TYPE_SERIALIZE(float, F32)
IMPLEMENT_BASE_TYPE_SERIALIZE(double, F64)
IMPLEMENT_BASE_TYPE_SERIALIZE(CharString, String)

#undef IMPLEMENT_BASE_TYPE_SERIALIZE

// Additional Metro types serialization
inline void operator >>(MetroReflectionStream& s, Bool8& v) {
    s >> v.val8;
}

inline void operator >>(MetroReflectionStream& s, flags32& v) {
    s >> v.value;
}

inline void operator >>(MetroReflectionStream& s, ivec4& v) {
    s.SerializeInts(&v.x, 4);
}

inline void operator >>(MetroReflectionStream& s, vec2& v) {
    s.SerializeFloats(&v.x, 2);
}

inline void operator >>(MetroReflectionStream& s, vec3& v) {
    s.SerializeFloats(&v.x, 3);
}

inline void operator >>(MetroReflectionStream& s, vec4& v) {
    s.SerializeFloats(&v.x, 4);
}

inline void operator >>(MetroReflectionStream& s, quat& v) {
    s.SerializeFloats(&v.x, 4);
}

inline void operator >>(MetroReflectionStream& s, color4f& v) {
    s.SerializeFloats(&v.r, 4);
}

inline void operator >>(MetroReflectionStream& s, color32u& v) {
    s >> v.value;
}

inline void operator >>(MetroReflectionStream& s, pose_43& v) {
    s.SerializeFloats(rcast<float*>(&v), sizeof(v) / sizeof(float));
}

inline void operator >>(MetroReflectionStream& s, pose_43T& v) {
    s.SerializeFloats(rcast<float*>(&v), sizeof(v) / sizeof(float));
}

inline void operator >>(MetroReflectionStream& s, anglef& v) {
    s >> v.x;
}

inline void operator >>(MetroReflectionStream& s, fp32_q8& v) {
    if (s.IsIn()) {
        uint8_t u8;
        s.SerializeU8(u8);
        v.SetAsU8(u8);
    } else {
        uint8_t u8 = v.GetAsU8();
        s.SerializeU8(u8);
    }
}

inline void operator >>(MetroReflectionStream& s, EntityLink& v) {
    s >> v.value;
}

inline void operator >>(MetroReflectionStream& s, vec4s16& v) {
    s.SerializeRawBytes(&v, sizeof(v));
}

inline void operator >>(MetroReflectionStream& s, ang3f& v) {
    s.SerializeFloats(&v.x, 3);
}

// Binary serialization
#include "MetroReflectionBinary.inl"
// Json serialization
#include "MetroReflectionJson.inl"




template <typename T>
struct ArrayElementTypeGetter {
    typedef typename T::value_type elem_type;
};


#define CLEAN_TYPE(typeToClean) std::remove_reference<decltype(typeToClean)>::type


#define METRO_SERIALIZE_MEMBER_NO_VERIFY(s, memberName)  s >> memberName;

#define METRO_SERIALIZE_MEMBER(s, memberName)                                                       \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());      \
    (s) >> memberName;

#define METRO_SERIALIZE_NAMED_MEMBER(s, memberName, memberDataName)                                 \
    (s).SerializeTypeInfo(STRINGIFY(memberDataName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_STRUCT_MEMBER(s, memberName) (s).SerializeStruct(STRINGIFY(memberName), memberName)

#define METRO_SERIALIZE_ARRAY_MEMBER(s, memberName)                                                                                     \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeArrayGetAlias<ArrayElementTypeGetter<decltype(memberName)>::elem_type>());    \
    (s).SerializeArray<ArrayElementTypeGetter<decltype(memberName)>::elem_type, uint32_t>(memberName);

#define METRO_SERIALIZE_ARRAY_16_MEMBER(s, memberName)                                                                                  \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeArrayGetAlias<ArrayElementTypeGetter<decltype(memberName)>::elem_type>());    \
    (s).SerializeArray<ArrayElementTypeGetter<decltype(memberName)>::elem_type, uint16_t>(memberName);

#define METRO_SERIALIZE_STR_ARRAY_MEMBER(s, memberName) {               \
        (s).SerializeTypeInfo(STRINGIFY(memberName), "str_array32");    \
        uint32_t str_array32_size = scast<uint32_t>(memberName.size()); \
        METRO_SERIALIZE_NAMED_MEMBER(s, str_array32_size, memberName);  \
        if (s.IsIn()) {                                                 \
            memberName.resize(str_array32_size);                        \
        }                                                               \
        for (auto& ss : memberName) {                                   \
            METRO_SERIALIZE_NAMED_MEMBER(s, ss, memberName);            \
        }                                                               \
    }

#define METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(s, memberName) (s).SerializeStructArray(STRINGIFY(memberName), memberName)

#define METRO_SERIALIZE_MEMBER_CHOOSE(s, memberName)                                            \
    (s).SerializeEditorTag(STRINGIFY(memberName));                                              \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_ANIMSTR(s, memberName)                                           \
    (s).SerializeEditorTag(STRINGIFY(memberName), 1);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_PARTSTR(s, memberName)                                           \
    (s).SerializeEditorTag(STRINGIFY(memberName), 2);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_SOUNDSTR(s, memberName)                                          \
    (s).SerializeEditorTag(STRINGIFY(memberName), 3);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_FLAGS8(s, memberName)                                            \
    (s).SerializeEditorTag(STRINGIFY(memberName), 4);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_FLAGS64(s, memberName)                                           \
    (s).SerializeEditorTag(STRINGIFY(memberName), 5);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_ATTP_STR(s, memberName)                                           \
    (s).SerializeEditorTag(STRINGIFY(memberName), 6);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_FLAGS32(s, memberName)                                           \
    (s).SerializeEditorTag(STRINGIFY(memberName), 7);                                           \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CLEAN_TYPE(memberName)>());  \
    (s) >> memberName;

#define METRO_SERIALIZE_MEMBER_STRARRAY_CHOOSE(s, memberName)                       \
    (s).SerializeEditorTag(STRINGIFY(memberName));                                  \
    (s).SerializeTypeInfo(STRINGIFY(memberName), MetroTypeGetAlias<CharString>());  \
    (s) >> memberName;


#define METRO_SERIALIZE_BASE_CLASS(s)  Base::Serialize(s)
