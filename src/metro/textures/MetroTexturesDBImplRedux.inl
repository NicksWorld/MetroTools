namespace ReduxImpl {

struct MetroTextureInfo {
    CharString  bump_name;
    CharString  det_name;
    CharString  aux0_name;
    CharString  aux1_name;
    uint8_t     type;
    uint32_t    width;
    uint32_t    height;
    float       bump_height;
    uint8_t     displ_type;
    float       displ_height;
    bool        animated;
    bool        mipmapped;
    bool        streamable;
    uint8_t     priority;
    float       det_u_scale;
    float       det_v_scale;
    float       det_intesity;
    uint8_t     format;
    color32u    avg_color;

    // transient, helpers
    bool        crunched;

    void Serialize(MetroReflectionStream& s) {
        METRO_SERIALIZE_MEMBER(s, bump_name);
        METRO_SERIALIZE_MEMBER(s, det_name);
        METRO_SERIALIZE_MEMBER(s, aux0_name);
        METRO_SERIALIZE_MEMBER(s, aux1_name);
        METRO_SERIALIZE_MEMBER(s, type);
        METRO_SERIALIZE_MEMBER(s, width);
        METRO_SERIALIZE_MEMBER(s, height);
        METRO_SERIALIZE_MEMBER(s, bump_height);
        METRO_SERIALIZE_MEMBER(s, displ_type);
        METRO_SERIALIZE_MEMBER(s, displ_height);
        METRO_SERIALIZE_MEMBER(s, animated);
        METRO_SERIALIZE_MEMBER(s, mipmapped);
        METRO_SERIALIZE_MEMBER(s, streamable);
        METRO_SERIALIZE_MEMBER(s, priority);
        METRO_SERIALIZE_MEMBER(s, det_u_scale);
        METRO_SERIALIZE_MEMBER(s, det_v_scale);
        METRO_SERIALIZE_MEMBER(s, det_intesity);
        METRO_SERIALIZE_MEMBER(s, format);
        METRO_SERIALIZE_MEMBER(s, avg_color);
    }
};

class MetroTexturesDBImplRedux final : public MetroTexturesDBImpl {
public:
    MetroTexturesDBImplRedux() {}
    virtual ~MetroTexturesDBImplRedux() {}

    virtual bool Initialize(const fs::path& binPath) override {
        bool result = false;

        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
        MemStream stream = binPath.empty() ? mfs.OpenFileFromPath(R"(content\scripts\texture_aliases.bin)") : OSReadFile(binPath);
        if (stream) {
            result = this->LoadAliases(stream);
        }

        return result;
    }

    virtual const CharString& GetSourceName(const HashString& name) const override {
        const HashString& alias = this->GetAlias(name);
        return alias.Valid() ? alias.str : name.str;
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

    virtual const CharString& GetAuxName(const HashString&, const size_t) const override {
        return kEmptyString;
    }

    virtual StringArray GetAllLevels(const HashString& name) const override {
        StringArray result;

        const CharString& realName = this->GetSourceName(name);
        const MetroTextureInfo* mti = this->GetInfoByName(realName);
        if (mti) {
            if (!mti->streamable) {
                result.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(realName, *mti));
            } else {
                this->CollectAllLevelsWithExt(realName, *mti, result);
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
            CharString bumpName = mti->bump_name;
            CharString detName = mti->det_name;

            result.albedo = realName;
            if (!allMips || !mti->streamable) {
                result.albedoPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(realName, *mti));
            } else {
                this->CollectAllLevelsWithExt(realName, *mti, result.albedoPaths);
            }

            const MetroTextureInfo* mtiBump;
            if (!bumpName.empty() && (mtiBump = this->GetInfoByName(bumpName), mtiBump != nullptr)) {
                result.bump = bumpName;
                if (!allMips || !mtiBump->streamable) {
                    result.bumpPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(result.bump, *mtiBump));
                } else {
                    this->CollectAllLevelsWithExt(result.bump, *mtiBump, result.bumpPaths);
                }
            }

            const MetroTextureInfo* mtiDetail;
            if (!detName.empty() && (mtiDetail = this->GetInfoByName(detName), mtiDetail != nullptr)) {
                result.detail = detName;
                if (!allMips || !mtiDetail->streamable) {
                    result.detailPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(result.detail, *mtiDetail));
                } else {
                    this->CollectAllLevelsWithExt(result.detail, *mtiDetail, result.detailPaths);
                }
            }
        }

        return std::move(result);
    }

private:
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

    //#NOTE_SK: I know this is a shitty code, but it's to be compatible with other implementations
    //          so this method has to be constant :(
    MetroTextureInfo* LoadTextureInfo(const HashString& textureName) const {
        MetroTextureInfo* result = nullptr;

        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

        CharString binPath = MetroFileSystem::Paths::TexturesFolder + textureName.str + ".bin";
        MemStream stream = mfs.OpenFileFromPath(binPath);
        if (stream) {
            MetroBinArchive bin(kEmptyString, stream, MetroBinArchive::kHeaderNotExist);
            StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

            const size_t oldSize = mPool.size();
            mPool.resize(oldSize + 1);
            MetroTextureInfo& texInfo = mPool.back();
            (*reader) >> texInfo;

            //#NOTE_SK: at the moment I couldn't find a proper way to determine if a texture
            //          was crunched or not, so using filesystem query for that :(
            if (texInfo.streamable) {
                const CharString queryPath = MetroFileSystem::Paths::TexturesFolder + textureName.str + ".512c";
                const MetroFSPath& queryFileHandle = mfs.FindFile(queryPath);
                texInfo.crunched = queryFileHandle.IsValid();
            } else {
                texInfo.crunched = false;
            }

            mDatabase.insert({ textureName, oldSize });

            result = &texInfo;
        }

        return result;
    }

    const HashString& GetAlias(const HashString& name) const {
        auto it = mAliases.find(name);
        if (it == mAliases.end()) {
            return kEmptyHashString;
        } else {
            return it->second;
        }
    }

    const MetroTextureInfo* GetInfoByName(const HashString& name) const {
        const auto it = mDatabase.find(name);
        if (it == mDatabase.end()) {
            return this->LoadTextureInfo(name);
        } else {
            return &mPool[it->second];
        }
    }

    CharString GetNameWithExt(const CharString& name, const MetroTextureInfo& mti) const {
        if (mti.streamable) {
            CharString result = name + '.' + std::to_string(mti.width);
            if (mti.crunched) {
                result.push_back('c');
            }
            return std::move(result);
        } else {
            return name + ".dds";
        }
    }

    void CollectAllLevelsWithExt(const CharString& name, const MetroTextureInfo& mti, StringArray& dest) const {
        size_t width = mti.width;
        while (width >= 512) {
            dest.push_back(MetroFileSystem::Paths::TexturesFolder + name + '.' + std::to_string(width) + (mti.crunched ? "c" : ""));
            width >>= 1;
        }
    }

private:
    mutable MyArray<MetroTextureInfo>   mPool;
    mutable MyDict<HashString, size_t>  mDatabase;
    MyDict<HashString, HashString>      mAliases;
};

} // namespace ReduxImpl
