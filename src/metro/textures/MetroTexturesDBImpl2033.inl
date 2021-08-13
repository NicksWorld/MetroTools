namespace Metro2033Impl {

    struct MetroTextureInfo {
        int32_t     type;
        bool        animated;
        uint32_t    fmt;
        uint32_t    r_width;
        uint32_t    r_height;
        CharString  name;
        CharString  bump_name;
        float       bump_height;
        uint8_t     parr_height;
        CharString  det_name;
        float       det_u_scale;
        float       det_v_scale;
        float       det_int;
        bool        mip_enabled;
        bool        streamable;
        bool        priority;
        uint32_t    avg_color;

        void Serialize(MetroReflectionStream& s) {
            METRO_SERIALIZE_MEMBER(s, type);
            METRO_SERIALIZE_MEMBER(s, animated);
            METRO_SERIALIZE_MEMBER(s, fmt);
            METRO_SERIALIZE_MEMBER(s, r_width);
            METRO_SERIALIZE_MEMBER(s, r_height);
            METRO_SERIALIZE_MEMBER(s, name);
            METRO_SERIALIZE_MEMBER(s, bump_name);
            METRO_SERIALIZE_MEMBER(s, bump_height);
            METRO_SERIALIZE_MEMBER(s, parr_height);
            METRO_SERIALIZE_MEMBER(s, det_name);
            METRO_SERIALIZE_MEMBER(s, det_u_scale);
            METRO_SERIALIZE_MEMBER(s, det_v_scale);
            METRO_SERIALIZE_MEMBER(s, det_int);
            METRO_SERIALIZE_MEMBER(s, mip_enabled);
            METRO_SERIALIZE_MEMBER(s, streamable);
            METRO_SERIALIZE_MEMBER(s, priority);
            METRO_SERIALIZE_MEMBER(s, avg_color);
        }
    };

    class MetroTexturesDBImpl2033 final : public MetroTexturesDBImpl {
    public:
        MetroTexturesDBImpl2033() {}
        virtual ~MetroTexturesDBImpl2033() {}

        bool Initialize(const fs::path& binPath) override {
            bool result = false;

            const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
            MemStream stream = binPath.empty() ? mfs.OpenFileFromPath(R"(content\textures\textures.bin)") : OSReadFile(binPath);
            if (stream) {
                result = this->LoadDB(stream);
            }

            return result;
        }

        bool SaveBin(const fs::path& binPath) override {
            MemWriteStream stream;
            MetroBinArchive bin(stream);
            MetroReflectionBinaryWriteStream reflStream(stream, MetroReflectionFlags::DefaultOutFlags);
            reflStream.SetSTable(bin.GetSTable());

            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(reflStream, texture_params);

            bin.Finalize();

            const size_t written = OSWriteFile(binPath, stream.Data(), stream.GetWrittenBytesCount());

            return written == stream.GetWrittenBytesCount();
        }

        const CharString& GetSourceName(const HashString& name) const override {
            return name.str;
        }

        const CharString& GetBumpName(const HashString& name) const override {
            const CharString& realName = this->GetSourceName(name);
            const MetroTextureInfo* mti = this->GetInfoByName(realName);
            return mti ? mti->bump_name : kEmptyString;
        }

        const CharString& GetDetName(const HashString& name) const override {
            const CharString& realName = this->GetSourceName(name);
            const MetroTextureInfo* mti = this->GetInfoByName(realName);
            return mti ? mti->det_name : kEmptyString;
        }

        const CharString& GetAuxName(const HashString&, const size_t) const override {
            return kEmptyString;
        }

        StringArray GetAllLevels(const HashString& name) const override {
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

        MetroSurfaceDescription GetSurfaceSetFromFile(const MetroFSPath& file, const bool allMips) const override {
            CharString fullPath = MetroContext::Get().GetFilesystem().GetFullPath(file);
            CharString relativePath = fullPath.substr(MetroFileSystem::Paths::TexturesFolder.length());

            // remove extension
            const CharString::size_type dotPos = relativePath.find_last_of('.');
            if (dotPos != CharString::npos) {
                relativePath = relativePath.substr(0, dotPos);
            }

            return this->GetSurfaceSetFromName(relativePath, allMips);
        }

        MetroSurfaceDescription GetSurfaceSetFromName(const HashString& textureName, const bool allMips) const override {
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
            }

            return std::move(result);
        }

        size_t GetNumTextures() const override {
            return texture_params.size();
        }

        const CharString& GetTextureNameByIdx(const size_t idx) const override {
            return texture_params[idx].name;
        }

        void FillCommonInfoByIdx(const size_t idx, MetroTextureInfoCommon& info) const override {
            const MetroTextureInfo& srcInfo = texture_params[idx];

            info.type           = srcInfo.type;
            info.animated       = srcInfo.animated;
            info.fmt            = srcInfo.fmt;
            info.r_width        = srcInfo.r_width;
            info.r_height       = srcInfo.r_height;
            info.name           = srcInfo.name;
            info.bump_name      = srcInfo.bump_name;
            info.bump_height    = srcInfo.bump_height;
            info.parr_height    = srcInfo.parr_height;
            info.det_name       = srcInfo.det_name;
            info.det_u_scale    = srcInfo.det_u_scale;
            info.det_v_scale    = srcInfo.det_v_scale;
            info.det_int        = srcInfo.det_int;
            info.mip_enabled    = srcInfo.mip_enabled;
            info.streamable     = srcInfo.streamable;
            info.priority       = srcInfo.priority;
            info.avg_color      = srcInfo.avg_color;
        }

        void SetCommonInfoByIdx(const size_t idx, const MetroTextureInfoCommon& info) override {
            MetroTextureInfo& srcInfo = texture_params[idx];

            srcInfo.type           = info.type;
            srcInfo.animated       = info.animated;
            srcInfo.fmt            = info.fmt;
            srcInfo.r_width        = info.r_width;
            srcInfo.r_height       = info.r_height;
            srcInfo.name           = info.name;
            srcInfo.bump_name      = info.bump_name;
            srcInfo.bump_height    = info.bump_height;
            srcInfo.parr_height    = info.parr_height;
            srcInfo.det_name       = info.det_name;
            srcInfo.det_u_scale    = info.det_u_scale;
            srcInfo.det_v_scale    = info.det_v_scale;
            srcInfo.det_int        = info.det_int;
            srcInfo.mip_enabled    = info.mip_enabled;
            srcInfo.streamable     = info.streamable;
            srcInfo.priority       = info.priority;
            srcInfo.avg_color      = info.avg_color;
        }

        void RemoveTextureByIdx(const size_t idx) override {
            mDatabase.erase(HashString(texture_params[idx].name));
            texture_params.erase(texture_params.begin() + idx);

            // fix the dictionary
            for (auto& v : mDatabase) {
                if (v.second > idx) {
                    v.second--;
                }
            }
        }

        void AddTexture(const MetroTextureInfoCommon& info) override {
            MetroTextureInfo newInfo;

            newInfo.type           = info.type;
            newInfo.animated       = info.animated;
            newInfo.fmt            = info.fmt;
            newInfo.r_width        = info.r_width;
            newInfo.r_height       = info.r_height;
            newInfo.name           = info.name;
            newInfo.bump_name      = info.bump_name;
            newInfo.bump_height    = info.bump_height;
            newInfo.parr_height    = info.parr_height;
            newInfo.det_name       = info.det_name;
            newInfo.det_u_scale    = info.det_u_scale;
            newInfo.det_v_scale    = info.det_v_scale;
            newInfo.det_int        = info.det_int;
            newInfo.mip_enabled    = info.mip_enabled;
            newInfo.streamable     = info.streamable;
            newInfo.priority       = info.priority;
            newInfo.avg_color      = info.avg_color;

            texture_params.push_back(newInfo);
            mDatabase.insert({ newInfo.name, texture_params.size() - 1 });
        }

    private:
        bool LoadDB(MemStream& stream) {
            bool result = false;

            MetroBinArchive bin("textures.bin", stream, MetroBinArchive::kHeaderDoAutoSearch);
            StrongPtr<MetroReflectionStream> reader = bin.ReflectionReader();

            METRO_SERIALIZE_STRUCT_ARRAY_MEMBER(*reader, texture_params);
            const size_t numTextures = texture_params.size();
            mDatabase.reserve(numTextures);
            for (size_t i = 0; i < numTextures; ++i) {
                const MetroTextureInfo& ti = texture_params[i];
                mDatabase.insert({ ti.name, i });
            }

            result = !mDatabase.empty();

            return result;
        }

        const MetroTextureInfo* GetInfoByName(const HashString& name) const {
            const auto it = mDatabase.find(name);
            if (it == mDatabase.end()) {
                return nullptr;
            } else {
                return &texture_params[it->second];
            }
        }

        CharString GetNameWithExt(const CharString&, const MetroTextureInfo& mti) const {
            return mti.name + ((mti.streamable) ? (CharString(".") + std::to_string(mti.r_width)) : ".dds");
        }

        void CollectAllLevelsWithExt(const CharString& name, const MetroTextureInfo& mti, StringArray& dest) const {
            size_t width = mti.r_width;
            while (width >= 512) {
                dest.push_back(MetroFileSystem::Paths::TexturesFolder + name + '.' + std::to_string(width));
                width >>= 1;
            }
        }

    private:
        MyArray<MetroTextureInfo>       texture_params;
        MyDict<HashString, size_t>      mDatabase;
    };

} // namespace Metro2033Impl
