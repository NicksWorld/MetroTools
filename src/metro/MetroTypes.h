#pragma once
#include "mycommon.h"
#include "mymath.h"

enum class MetroGameVersion {
    OG2033,         // Metro 2033 (Original 2010)
    OGLastLight,    // Metro Last Light (Original 2013)
    Redux,          // Metro 2033/Last Ligt (Redux 2014, Switch 2020)
    Arktika1,       // Arktika.1 (2017) [engine is very close to that used in Exodus]
    Exodus,         // Metro Exodus (2019)
    Unknown
};

PACKED_STRUCT_BEGIN
struct MetroGuid {  // 16 bytes
    uint32_t    a;
    uint16_t    b;
    uint16_t    c;
    uint16_t    d;
    uint8_t     e[6];

    inline bool operator ==(const MetroGuid& other) const {
        return this->a == other.a && this->b == other.b && this->c == other.c && this->d == other.d &&
               this->e[0] == other.e[0] && this->e[1] == other.e[1] && this->e[2] == other.e[2] &&
               this->e[3] == other.e[3] && this->e[4] == other.e[4] && this->e[5] == other.e[5];
    }

    inline bool operator !=(const MetroGuid& other) const {
        return !(*this == other);
    }
} PACKED_STRUCT_END;

struct MetroFile {
public:
    enum : size_t {
        Flag_Unknown4   = 4,    // patch folders have this one
        Flag_Folder     = 8,
    };

public:
    struct iterator {
        friend MetroFile;
    private:
        iterator(const size_t _idx) : idx(_idx) {}

    public:
        inline size_t operator *() const {
            return this->idx;
        }
        inline bool operator ==(const iterator& other) {
            return this->idx == other.idx;
        }
        inline bool operator !=(const iterator& other) {
            return this->idx != other.idx;
        }
        inline iterator& operator ++() {
            ++this->idx;
            return *this;
        }
        inline iterator operator ++(int) {
            iterator prev(this->idx);
            ++this->idx;
            return prev;
        }

    private:
        size_t idx;
    };

public:
    static const size_t InvalidFileIdx = kInvalidValue;

    inline bool IsFile() const {
        return !TestBit<size_t>(this->flags, Flag_Folder);
    }

    inline bool ContainsFile(const size_t fileIdx) const {
        return this->IsFile() ? false : (fileIdx >= this->firstFile && fileIdx < (this->firstFile + this->numFiles));
    }

    //#NOTE_SK: iteration over files
    //          using standard names so that modern for(i : a) syntaxis works
    iterator begin() const {
        return (this->IsFile() ? iterator(kInvalidValue) : iterator(this->firstFile));
    }
    iterator end() const {
        return (this->IsFile() ? iterator(kInvalidValue) : iterator(this->firstFile + this->numFiles));
    }

    // common fields
    size_t      idx;
    size_t      flags;
    CharString  name;

    // file fields
    size_t      pakIdx;
    size_t      offset;
    size_t      sizeUncompressed;
    size_t      sizeCompressed;

    // dir fields
    size_t      firstFile;
    size_t      numFiles;

    // duplication
    size_t      baseIdx;
    size_t      duplicates;
};


PACKED_STRUCT_BEGIN
struct MetroFace {
    uint16_t a, b, c;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct MetroVertex {
    vec3        pos;
    uint8_t     bones[4];
    vec4        normal;
    vec4        tangent;
    uint8_t     weights[4];
    vec2        uv0;
    vec2        uv1;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct MetroOBB {           // size = 60
    mat3    matrix;
    vec3    offset;
    vec3    hsize;
} PACKED_STRUCT_END;

struct MetroMesh {
    size_t               version;
    bool                 skinned;
    bool                 isCollision;
    size_t               flags;
    float                vscale;
    AABBox               bbox;
    size_t               type;
    size_t               shaderId;
    StringArray          materials;
    MyArray<MetroFace>   faces;
    BytesArray           rawVB;
    size_t               numVertices;
    BytesArray           bonesRemap;
    MyArray<MetroOBB>    hitBoxes;
};


struct MetroVertexType {
    enum VertexType : uint32_t {
        Invalid     = 0,
        Skin        = 1,
        Static      = 2,
        Level       = 3,
        LevelLegacy = 4,

        Particle    = 16,
        Soft        = 17,
        Impostor    = 19,

        SkinNB      = 25
    };
};

// all these vertices are 32 bytes each
PACKED_STRUCT_BEGIN
struct VertexStatic {
    vec3     pos;
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    vec2     uv;
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct VertexSkinned {
    int16_t  pos[4];
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    uint8_t  bones[4];
    uint8_t  weights[4];
    int16_t  uv[2];
} PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN
struct VertexLevel {
    vec3     pos;
    uint32_t normal;
    uint32_t aux0;
    uint32_t aux1;
    int16_t  uv0[2];
    int16_t  uv1[2];
} PACKED_STRUCT_END;

// 16 bytes, my own vertex for terrain
PACKED_STRUCT_BEGIN
struct VertexTerrain {
    vec3     pos;
    uint16_t uv0[2];    // [0.0 to 1.0] in [0 to 65535] range
} PACKED_STRUCT_END;


static vec4 DecodeNormal(const uint32_t n) {
    const float div255 = 1.0f / 255.0f;
    const float x = (scast<float>((n & 0x00FF0000) >> 16) * div255) * 2.0f - 1.0f;
    const float y = (scast<float>((n & 0x0000FF00) >>  8) * div255) * 2.0f - 1.0f;
    const float z = (scast<float>((n & 0x000000FF) >>  0) * div255) * 2.0f - 1.0f;
    const float w = (scast<float>((n & 0xFF000000) >> 24) * div255); // vao

    return vec4(x, y, z, w);
}

static uint32_t EncodeNormal(const vec3& normal, const float vao) {
    const vec3 remappedN = normal * 0.5f + 0.5f;
    const float clampedVao = Clamp(vao, 0.0f, 1.0f);

    const uint32_t uX = scast<uint32_t>(Clamp(remappedN.x * 255.0f, 0.0f, 255.0f));
    const uint32_t uY = scast<uint32_t>(Clamp(remappedN.y * 255.0f, 0.0f, 255.0f));
    const uint32_t uZ = scast<uint32_t>(Clamp(remappedN.z * 255.0f, 0.0f, 255.0f));
    const uint32_t uVao = scast<uint32_t>(Clamp(clampedVao * 255.0f, 0.0f, 255.0f));

    const uint32_t result = (uVao & 0xFF) << 24 |
                            (uX   & 0xFF) << 16 |
                            (uY   & 0xFF) <<  8 |
                            (uZ   & 0xFF);
    return result;
}

static vec4 DecodeTangent(const uint32_t n) {
    const float div = 1.0f / 127.5f;
    const float x = (((n & 0x00FF0000) >> 16) * div) - 1.0f;
    const float y = (((n & 0x0000FF00) >> 8) * div) - 1.0f;
    const float z = (((n & 0x000000FF) >> 0) * div) - 1.0f;
    const float w = (((n & 0xFF000000) >> 24) * div) - 1.0f;    // sign ?

    return vec4(x, y, z, w);
}

inline vec3 MetroSwizzle(const vec3& v) {
    return vec3(v.z, v.y, v.x);
}

inline vec4 MetroSwizzle(const vec4& v) {
    return vec4(v.z, v.y, v.x, v.w);
}

inline quat MetroSwizzle(const quat& q) {
    return quat(q.w, q.z, q.y, q.x);
}

inline void MetroSwizzle(uint8_t* a) {
    std::swap(a[0], a[2]);
}

template <typename T>
inline MetroVertex ConvertVertex(const T&) {
    assert(false);
}

template <>
inline MetroVertex ConvertVertex<VertexStatic>(const VertexStatic& v) {
    MetroVertex result = {};

    result.pos = MetroSwizzle(v.pos);
    result.normal = MetroSwizzle(DecodeNormal(v.normal));
    result.tangent = MetroSwizzle(DecodeTangent(v.aux0));
    result.uv0 = v.uv;

    return result;
}

template <>
inline MetroVertex ConvertVertex<VertexSkinned>(const VertexSkinned& v) {
    const float posDequant = 1.0f / 32767.0f;
    const float uvDequant = 1.0f / 2048.0f;

    MetroVertex result = {};

    result.pos = MetroSwizzle(vec3(scast<float>(v.pos[0]) * posDequant,
                                   scast<float>(v.pos[1]) * posDequant,
                                   scast<float>(v.pos[2]) * posDequant));
    *rcast<uint32_t*>(result.bones) = *rcast<const uint32_t*>(v.bones);
    MetroSwizzle(result.bones);
    result.normal = MetroSwizzle(DecodeNormal(v.normal));
    result.tangent = MetroSwizzle(DecodeTangent(v.aux0));
    *rcast<uint32_t*>(result.weights) = *rcast<const uint32_t*>(v.weights);
    MetroSwizzle(result.weights);
    result.uv0 = vec2(scast<float>(v.uv[0]) * uvDequant,
                      scast<float>(v.uv[1]) * uvDequant);
    return result;
}

template <>
inline MetroVertex ConvertVertex<VertexLevel>(const VertexLevel& v) {
    const float kUV0Dequant = 1.0f / 1024.0f;
    const float kUV1Dequant = 1.0f / 32767.0f;

    MetroVertex result = {};

    result.pos = MetroSwizzle(v.pos);
    result.normal = MetroSwizzle(DecodeNormal(v.normal));
    result.tangent = MetroSwizzle(DecodeTangent(v.aux0));
    result.uv0 = vec2(scast<float>(v.uv0[0]) * kUV0Dequant,
                      scast<float>(v.uv0[1]) * kUV0Dequant);
    result.uv1 = vec2(scast<float>(v.uv1[0]) * kUV1Dequant,
                      scast<float>(v.uv1[1]) * kUV1Dequant);
    return result;
}


enum class MetroBodyPart : size_t {
    Invalid     = 0,
    Generic     = 1,
    Head        = 2,
    Stomach     = 3,
    Leftarm     = 4,
    Rightarm    = 5,
    Leftleg     = 6,
    Rightleg    = 7,
    Chest       = 8,
    Gear        = 9
};

enum class MetroLanguage : size_t {
    CN = 0,
    CZ,
    DE,
    ES,
    FR,
    IT,
    JP,
    KO,
    MX,
    PL,
    PT,
    RU,
    TW,
    UK,
    US,

    First = CN,
    Last = US,
    Count = Last + 1
};

static const CharString MetroLanguagesStr[scast<size_t>(MetroLanguage::Count)] = {
    "cn",
    "cz",
    "de",
    "es",
    "fr",
    "it",
    "jp",
    "ko",
    "mx",
    "pl",
    "pt",
    "ru",
    "tw",
    "uk",
    "us"
};

static const CharString MetroLanguagesFullName[scast<size_t>(MetroLanguage::Count)] = {
    "Chinese",
    "Czech",
    "German",
    "Spanish",
    "French",
    "Italian",
    "Japanese",
    "Korean",
    "Mexican",
    "Polish",
    "Portuguese",
    "Russian",
    "Taiwanese",
    "Ukrainian",
    "English"
};

struct MetroSurfaceDescription {
    CharString  albedo;
    StringArray albedoPaths;
    CharString  bump;
    StringArray bumpPaths;
    CharString  normalmap;
    StringArray normalmapPaths;
    CharString  detail;
    StringArray detailPaths;
};
