namespace ExodusImpl {

struct MetroTextureInfo {
    CharString          name;           //#NOTE_SK: transient member !!!

    uint32_t            type;           //#NOTE_SK: TextureType enum
    uint8_t             texture_type;   //#NOTE_SK: seems to be same as type
    CharString          source_name;
    vec4                surf_xform;
    uint32_t            format;         //#NOTE_SK: PixelFormat enum
    uint32_t            width;
    uint32_t            height;
    bool                animated;
    bool                draft;
    bool                override_avg_color;
    color4f             avg_color;
    CharString          shader_name;    // choose
    CharString          gamemtl_name;   // choose
    uint32_t            priority;
    bool                streamable;
    float               bump_height;
    uint8_t             displ_type;     //#NOTE_SK: DisplType enum
    float               displ_height;
    float               parallax_height_mul;
    bool                mipmapped;
    float               reflectivity;
    bool                treat_as_metal;
    CharString          det_name;       // choose
    float               det_scale_u;
    float               det_scale_v;
    float               det_intensity;
    color4f             aux_params;
    color4f             aux_params_1;
    // !!! Optional fields !!!
    MyArray<uint8_t>    lum;
    MyArray<float>      sph_coefs;
    ///////////////////////////
    CharString          bump_name;      // choose
    CharString          aux0_name;      // choose
    CharString          aux1_name;      // choose
    CharString          aux2_name;      // choose
    CharString          aux3_name;      // choose
    CharString          aux4_name;      // choose
    CharString          aux5_name;      // choose
    CharString          aux6_name;      // choose
    CharString          aux7_name;      // choose

    void Serialize(MetroReflectionStream& s) {
        METRO_SERIALIZE_MEMBER(s, type);
        METRO_SERIALIZE_MEMBER(s, texture_type);
        METRO_SERIALIZE_MEMBER(s, source_name);
        METRO_SERIALIZE_MEMBER(s, surf_xform);
        METRO_SERIALIZE_MEMBER(s, format);
        METRO_SERIALIZE_MEMBER(s, width);
        METRO_SERIALIZE_MEMBER(s, height);
        METRO_SERIALIZE_MEMBER(s, animated);
        METRO_SERIALIZE_MEMBER(s, draft);
        METRO_SERIALIZE_MEMBER(s, override_avg_color);
        METRO_SERIALIZE_MEMBER(s, avg_color);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, shader_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, gamemtl_name);
        METRO_SERIALIZE_MEMBER(s, priority);
        METRO_SERIALIZE_MEMBER(s, streamable);
        METRO_SERIALIZE_MEMBER(s, bump_height);
        METRO_SERIALIZE_MEMBER(s, displ_type);
        METRO_SERIALIZE_MEMBER(s, displ_height);
        METRO_SERIALIZE_MEMBER(s, parallax_height_mul);
        METRO_SERIALIZE_MEMBER(s, mipmapped);
        METRO_SERIALIZE_MEMBER(s, reflectivity);
        METRO_SERIALIZE_MEMBER(s, treat_as_metal);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, det_name);
        METRO_SERIALIZE_MEMBER(s, det_scale_u);
        METRO_SERIALIZE_MEMBER(s, det_scale_v);
        METRO_SERIALIZE_MEMBER(s, det_intensity);
        METRO_SERIALIZE_MEMBER(s, aux_params);
        METRO_SERIALIZE_MEMBER(s, aux_params_1);

        // !!! Optional fields !!!
        if (this->texture_type == scast<uint8_t>(MetroTextureType::Cubemap_hdr)) {
            METRO_SERIALIZE_ARRAY_MEMBER(s, sph_coefs);
        } else if (this->texture_type == scast<uint8_t>(MetroTextureType::Unknown_has_lum)) {
            METRO_SERIALIZE_ARRAY_MEMBER(s, lum);
        }
        ///////////////////////////

        METRO_SERIALIZE_MEMBER_CHOOSE(s, bump_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux0_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux1_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux2_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux3_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux4_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux5_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux6_name);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, aux7_name);
    }
};


class MetroTexturesDBImplExodus final : public MetroTexturesDBImpl {
public:
    MetroTexturesDBImplExodus() {
    }
    virtual ~MetroTexturesDBImplExodus() {
    }

    virtual bool Initialize(const fs::path& binPath) override {
        bool result = false;

        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

        MemStream stream = binPath.empty() ? mfs.OpenFileFromPath(R"(content\textures_handles_storage.bin)") : OSReadFile(binPath);
        if (stream) {
            result = this->LoadHandles(stream);
        }

        if (result) {
            stream = mfs.OpenFileFromPath(R"(content\scripts\texture_aliases.bin)");
            if (stream) {
                result = this->LoadAliases(stream);
            }
        }

        return result;
    }

    virtual const CharString& GetSourceName(const HashString& name) const override {
        const HashString& alias = this->GetAlias(name);

        const MetroTextureInfo* mti = this->GetInfoByName(alias.Valid() ? alias : name);
        return (mti == nullptr) ? kEmptyString : mti->source_name;
    }

    virtual const CharString& GetBumpName(const HashString& name) const override {
        const CharString& realName = this->GetSourceName(name);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        return mti ? mti->bump_name : kEmptyString;
    }

    virtual const CharString& GetDetName(const HashString& name) const override {
        const CharString& realName = this->GetSourceName(name);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        return mti ? mti->det_name : kEmptyString;
    }

    virtual const CharString& GetAuxName(const HashString& name, const size_t) const override {
        const CharString& realName = this->GetSourceName(name);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        return mti ? mti->aux0_name : kEmptyString;
    }

    virtual StringArray GetAllLevels(const HashString& name) const override {
        StringArray result;

        const CharString& realName = this->GetSourceName(name);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        if (mti) {
            if (!mti->streamable) {
                result.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(*mti));
            } else {
                this->CollectAllLevelsWithExt(*mti, result);
            }
        }

        return std::move(result);
    }

    MetroTextureType GetTextyreType(const MetroFSPath& file) const override {
        CharString fullPath = MetroContext::Get().GetFilesystem().GetFullPath(file);
        CharString relativePath = fullPath.substr(MetroFileSystem::Paths::TexturesFolder.length());

        // remove extension
        const CharString::size_type dotPos = relativePath.find_last_of('.');
        if (dotPos != CharString::npos) {
            relativePath = relativePath.substr(0, dotPos);
        }

        const MetroTextureInfo* mti = this->GetInfoByName(relativePath);

        if (mti != nullptr) {
            return scast<MetroTextureType>(mti->type);
        }

        return MetroTextureType::Invalid;
    }

    virtual bool IsAlbedo(const MetroFSPath& file) const override {
        return GetTextyreType(file) == MetroTextureType::Diffuse;
    }

    virtual bool IsCubemap(const MetroFSPath& file) const override {
        const MetroTextureType textureType = GetTextyreType(file);

        return textureType == MetroTextureType::Cubemap || textureType == MetroTextureType::Cubemap_hdr;
    }

    virtual MetroSurfaceDescription GetSurfaceSetFromFile(const MetroFSPath& file, const bool allMips) const override {
        CharString fullPath = MetroContext::Get().GetFilesystem().GetFullPath(file);
        CharString relativePath = fullPath.substr(MetroFileSystem::Paths::TexturesFolder.length());

        // remove extension
        const CharString::size_type dotPos = relativePath.find_last_of('.');
        if (dotPos != CharString::npos) {
            relativePath = relativePath.substr(0, dotPos);
        }

        return this->GetSurfaceSetFromName(relativePath, allMips);
    }

    virtual MetroSurfaceDescription GetSurfaceSetFromName(const HashString& textureName, const bool allMips) const override {
        MetroSurfaceDescription result;

        const CharString& realName = this->GetSourceName(textureName);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        if (mti) {
            result.albedo = mti->source_name;
            if (!allMips || !mti->streamable) {
                result.albedoPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(*mti));
            } else {
                this->CollectAllLevelsWithExt(*mti, result.albedoPaths);
            }

            const MetroTextureInfo* mtiBump;
            if (!mti->bump_name.empty() && (mtiBump = this->GetInfoByName(mti->bump_name), mtiBump != nullptr)) {
                result.bump = mtiBump->source_name;
                if (!allMips || !mtiBump->streamable) {
                    result.bumpPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(*mtiBump));
                } else {
                    this->CollectAllLevelsWithExt(*mtiBump, result.bumpPaths);
                }

                const MetroTextureInfo* mtiNormalMap;
                if (!mtiBump->bump_name.empty() && (mtiNormalMap = this->GetInfoByName(mtiBump->bump_name), mtiNormalMap != nullptr)) {
                    result.normalmap = mtiNormalMap->source_name;
                    if (!allMips || !mtiNormalMap->streamable) {
                        result.normalmapPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(*mtiNormalMap));
                    } else {
                        this->CollectAllLevelsWithExt(*mtiNormalMap, result.normalmapPaths);
                    }
                }
            }

            const MetroTextureInfo* mtiDetail;
            if (!mti->det_name.empty() && (mtiDetail = this->GetInfoByName(mti->det_name), mtiDetail != nullptr)) {
                result.detail = mtiDetail->source_name;
                if (!allMips || !mtiDetail->streamable) {
                    result.detailPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(*mtiDetail));
                } else {
                    this->CollectAllLevelsWithExt(*mtiDetail, result.detailPaths);
                }
            }
        }

        return std::move(result);
    }

private:
    bool LoadHandles(MemStream& stream) {
        bool result = true;

        size_t numEntries = stream.ReadTyped<uint32_t>();
        if (numEntries == MakeFourcc<'A', 'V', 'E', 'R'>()) {
            stream.SkipBytes(2);
            numEntries = stream.ReadTyped<uint32_t>();
        }

        mPool.resize(numEntries);
        mDatabase.reserve(numEntries);

        for (size_t i = 0; i < numEntries; ++i) {
            const size_t idx = stream.ReadTyped<uint32_t>();
            const size_t size = stream.ReadTyped<uint32_t>();

            MemStream subStream = stream.Substream(size);

            CharString name = subStream.ReadStringZ();
            const uint8_t flags = subStream.ReadTyped<uint8_t>();

            MetroReflectionBinaryReadStream reader(subStream, flags);

            MetroTextureInfo* texInfo = &mPool[i];
            texInfo->name = name;
            reader >> *texInfo;

            HashString hashStr(texInfo->name);
            mDatabase.insert({ hashStr, texInfo });

            stream.SkipBytes(size);
        }

        return result;
    }

    bool LoadAliases(MemStream& stream) {
        bool result = false;

        MetroBinArchive bin("texture_aliases.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
        StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

        MyArray<MetroTextureAliasInfo> texture_aliases;
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, texture_aliases);

        if (!texture_aliases.empty()) {
            mAliases.reserve(texture_aliases.size());
            for (const auto& alias : texture_aliases) {
                mAliases.insert({ HashString(alias.src), HashString(alias.dst) });
            }

            result = true;
        }

        return result;
    }

    const HashString& GetAlias(const HashString& name) const {
        static HashString emptyString;

        auto it = mAliases.find(name);
        if (it == mAliases.end()) {
            return emptyString;
        }

        return it->second;
    }

    const MetroTextureInfo* GetInfoByName(const HashString& name) const {
        const auto it = mDatabase.find(name);
        if (it == mDatabase.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    CharString GetNameWithExt(const MetroTextureInfo& mti) const {
        return mti.source_name + ((mti.streamable) ? (CharString(".") + std::to_string(mti.width)) : ".dds");
    }

    void CollectAllLevelsWithExt(const MetroTextureInfo& mti, StringArray& dest) const {
        size_t width = mti.width;
        while (width >= 512) {
            dest.push_back(MetroFileSystem::Paths::TexturesFolder + mti.source_name + '.' + std::to_string(width));
            width >>= 1;
        }
    }

private:
    MyArray<MetroTextureInfo>             mPool;
    MyDict<HashString, MetroTextureInfo*> mDatabase;
    MyDict<HashString, HashString>        mAliases;
};

} // namespace ExodusImpl
