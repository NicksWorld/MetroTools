namespace LastLightImpl {

struct MetroTextureInfo {
    int32_t     type;
    bool        animated;
    uint32_t    fmt;
    uint32_t    r_width;
    uint32_t    r_height;
    CharString  name;       // choose
    CharString  bump_name;
    float       bump_height;
    uint8_t     displ_mode;
    uint8_t     parr_height;
    CharString  det_name;
    float       det_u_scale;
    float       det_v_scale;
    float       det_int;
    bool        mip_enabled;
    bool        streamable;
    bool        priority;
    uint32_t    avg_color;

    // transient, helpers
    volatile bool crunched;

    void Serialize(MetroReflectionStream& s) {
        METRO_SERIALIZE_MEMBER(s, type);
        METRO_SERIALIZE_MEMBER(s, animated);
        METRO_SERIALIZE_MEMBER(s, fmt);
        METRO_SERIALIZE_MEMBER(s, r_width);
        METRO_SERIALIZE_MEMBER(s, r_height);
        METRO_SERIALIZE_MEMBER_CHOOSE(s, name);
        METRO_SERIALIZE_MEMBER(s, bump_name);
        METRO_SERIALIZE_MEMBER(s, bump_height);
        METRO_SERIALIZE_MEMBER(s, displ_mode);
        METRO_SERIALIZE_MEMBER(s, parr_height);
        METRO_SERIALIZE_MEMBER(s, det_name);
        METRO_SERIALIZE_MEMBER(s, det_u_scale);
        METRO_SERIALIZE_MEMBER(s, det_v_scale);
        METRO_SERIALIZE_MEMBER(s, det_int);
        METRO_SERIALIZE_MEMBER(s, mip_enabled);
        METRO_SERIALIZE_MEMBER(s, streamable);
        METRO_SERIALIZE_MEMBER(s, priority);
        METRO_SERIALIZE_MEMBER(s, avg_color);

        crunched = true;
    }
};

class MetroTexturesDBImplLastLight final : public MetroTexturesDBImpl {
public:
    MetroTexturesDBImplLastLight() : mWorkerThread(INVALID_HANDLE_VALUE) {}
    virtual ~MetroTexturesDBImplLastLight() {
        if (mWorkerThread != INVALID_HANDLE_VALUE) {
            ::CloseHandle(mWorkerThread);
        }
    }

    virtual bool Initialize(const fs::path& binPath) override {
        bool result = true;

        const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
        MemStream stream = binPath.empty() ? mfs.OpenFileFromPath(R"(content\textures\textures.bin)") : ReadOSFile(binPath);
        if (stream) {
            result = this->LoadDB(stream);
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

    virtual bool IsAlbedo(const MyHandle file) const override {
        bool result = false;

        CharString fullPath = MetroContext::Get().GetFilesystem().GetFullPath(file);
        CharString relativePath = fullPath.substr(MetroFileSystem::Paths::TexturesFolder.length());

        // remove extension
        const CharString::size_type dotPos = relativePath.find_last_of('.');
        if (dotPos != CharString::npos) {
            relativePath = relativePath.substr(0, dotPos);
        }

        const MetroTextureInfo* mti = this->GetInfoByName(relativePath);
        result = (mti != nullptr && mti->type == scast<uint32_t>(MetroTextureType::Diffuse));

        return result;
    }

    virtual MetroSurfaceDescription GetSurfaceSetFromFile(const MyHandle file, const bool allMips) const override {
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
            result.albedo = realName;
            if (!allMips || !mti->streamable) {
                result.albedoPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(realName, *mti));
            } else {
                this->CollectAllLevelsWithExt(realName, *mti, result.albedoPaths);
            }

            const MetroTextureInfo* mtiBump;
            if (!mti->bump_name.empty() && (mtiBump = this->GetInfoByName(mti->bump_name), mtiBump != nullptr)) {
                result.bump = mti->bump_name;
                if (!allMips || !mtiBump->streamable) {
                    result.bumpPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(result.bump, *mtiBump));
                } else {
                    this->CollectAllLevelsWithExt(result.bump, *mtiBump, result.bumpPaths);
                }
            }

            const MetroTextureInfo* mtiDetail;
            if (!mti->det_name.empty() && (mtiDetail = this->GetInfoByName(mti->det_name), mtiDetail != nullptr)) {
                result.detail = mti->det_name;
                if (!allMips || !mtiDetail->streamable) {
                    result.detailPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(result.detail, *mtiDetail));
                } else {
                    this->CollectAllLevelsWithExt(result.detail, *mtiDetail, result.detailPaths);
                }
            }
        } else { // in case of broken LL build :)
            result.albedoPaths.push_back(MetroFileSystem::Paths::TexturesFolder + realName + ".512");
        }

        return std::move(result);
    }

private:
    static void ThreadFunc(void* argument) {
        MetroTexturesDBImplLastLight* _this = rcast<MetroTexturesDBImplLastLight*>(argument);
        _this->DetermineIfTexturesAreCrunched();
    }

    void DetermineIfTexturesAreCrunched() {
        LogPrint(LogLevel::Info, "LL crunched detector thread started...");

        for (MetroTextureInfo& mti : this->texture_params) {
            if (mti.streamable) {
                CharString fullName = MetroFileSystem::Paths::TexturesFolder + mti.name + ".512c";
                MyHandle file = MetroContext::Get().GetFilesystem().FindFile(fullName);
                if (kInvalidHandle == file) {
                    mti.crunched = false;
                }
            }
        }

        LogPrint(LogLevel::Info, "LL crunched detector thread ended!");
        mWorkerThread = INVALID_HANDLE_VALUE;
    }

    bool LoadDB(MemStream& stream) {
        bool result = false;

        MetroBinArchive bin("textures.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
        StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

        MyArray<MetroTextureAliasInfo> texture_aliases;
        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, texture_aliases);

        if (!texture_aliases.empty()) {
            mAliases.reserve(texture_aliases.size());
            for (const auto& alias : texture_aliases) {
                mAliases.insert({ HashString(alias.src), HashString(alias.dst) });
            }
        }

        METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, texture_params);
        const size_t numTextures = texture_params.size();
        mDatabase.reserve(numTextures);
        for (size_t i = 0; i < numTextures; ++i) {
            const MetroTextureInfo& ti = texture_params[i];
            mDatabase.insert({ ti.name, i });
        }

        result = !mDatabase.empty();

        //#NOTE_SK: sice I couldn't find any reliable way to determine if texture is crunched or not
        //          I'm spawning a worker thread here that'll check every single texture and set
        //          their 'crunched' flag accordingly
        mWorkerThread = (HANDLE)_beginthread(&MetroTexturesDBImplLastLight::ThreadFunc, 0, this);

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
            return nullptr;
        } else {
            return &texture_params[it->second];
        }
    }

    CharString GetNameWithExt(const CharString& name, const MetroTextureInfo& mti) const {
        if (mti.streamable) {
            CharString result = name + '.' + std::to_string(mti.r_width);
            if (mti.crunched) {
                result.push_back('c');
            }
            return std::move(result);
        } else {
            return name + ".dds";
        }
    }

    void CollectAllLevelsWithExt(const CharString& name, const MetroTextureInfo& mti, StringArray& dest) const {
        size_t width = mti.r_width;
        while (width >= 512) {
            dest.push_back(MetroFileSystem::Paths::TexturesFolder + name + '.' + std::to_string(width) + (mti.crunched ? "c" : ""));
            width >>= 1;
        }
    }

private:
    MyArray<MetroTextureInfo>       texture_params;
    MyDict<HashString, size_t>      mDatabase;
    MyDict<HashString, HashString>  mAliases;
    volatile HANDLE                 mWorkerThread;
};

} // namespace LastLightImpl
