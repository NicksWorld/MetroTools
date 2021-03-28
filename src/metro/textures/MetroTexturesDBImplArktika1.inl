namespace Arktika1Impl {

    struct MetroTextureInfo {
        uint32_t    type;           //#NOTE_SK: TextureType enum
        uint8_t     texture_type;   //#NOTE_SK: seems to be same as type
        CharString  source_name;
        uint32_t    format;
        uint32_t    width;
        uint32_t    height;
        vec4        avg_color;
        bool        animated;
        bool        draft;
        CharString  gamemtl_name;   // choose
        uint32_t    priority;
        bool        streamable;
        float       bump_height;    // in range [0.0099999998, 0.5]
        uint8_t     displ_type;
        float       displ_height;
        bool        mipmapped;
        float       reflectivity;
        bool        treat_as_metal;
        CharString  det_name;       // choose_array, str_shared
        float       det_scale_u;
        float       det_scale_v;
        float       det_intensity;
        color4f     aux_params;
        color4f     aux_params_1;
        CharString  bump_name;      // choose
        CharString  aux0_name;      // choose
        CharString  aux1_name;      // choose
        CharString  aux2_name;      // choose
        CharString  aux3_name;      // choose
        CharString  aux4_name;      // choose
        CharString  aux5_name;      // choose
        CharString  aux6_name;      // choose
        CharString  aux7_name;      // choose

        void Serialize(MetroReflectionStream& s) {
            METRO_SERIALIZE_MEMBER(s, type);
            METRO_SERIALIZE_MEMBER(s, texture_type);
            METRO_SERIALIZE_MEMBER(s, source_name);
            METRO_SERIALIZE_MEMBER(s, format);
            METRO_SERIALIZE_MEMBER(s, width);
            METRO_SERIALIZE_MEMBER(s, height);
            METRO_SERIALIZE_MEMBER(s, avg_color);
            METRO_SERIALIZE_MEMBER(s, animated);
            METRO_SERIALIZE_MEMBER(s, draft);
            METRO_SERIALIZE_MEMBER_CHOOSE(s, gamemtl_name);
            METRO_SERIALIZE_MEMBER(s, priority);
            METRO_SERIALIZE_MEMBER(s, streamable);
            METRO_SERIALIZE_MEMBER(s, bump_height);
            METRO_SERIALIZE_MEMBER(s, displ_type);
            METRO_SERIALIZE_MEMBER(s, displ_height);
            METRO_SERIALIZE_MEMBER(s, mipmapped);
            METRO_SERIALIZE_MEMBER(s, reflectivity);
            METRO_SERIALIZE_MEMBER(s, treat_as_metal);
            METRO_SERIALIZE_MEMBER_CHOOSE(s, det_name);
            METRO_SERIALIZE_MEMBER(s, det_scale_u);
            METRO_SERIALIZE_MEMBER(s, det_scale_v);
            METRO_SERIALIZE_MEMBER(s, det_intensity);
            METRO_SERIALIZE_MEMBER(s, aux_params);
            METRO_SERIALIZE_MEMBER(s, aux_params_1);
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

    class MetroTexturesDBImplArktika1 final : public MetroTexturesDBImpl {
    public:
        MetroTexturesDBImplArktika1() {}
        virtual ~MetroTexturesDBImplArktika1() {}

        virtual bool Initialize(const fs::path& binPath) override {
            bool result = false;

            const MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();

            MemStream stream = binPath.empty() ? mfs.OpenFileFromPath(R"(content\scripts\texture_aliases.bin)") : ReadOSFile(binPath);
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
            result = (mti != nullptr && mti->texture_type == scast<uint32_t>(MetroTextureType::Diffuse));

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

                    CharString normalmapName = mtiBump->bump_name;

                    const MetroTextureInfo* mtiNormalMap;
                    if (!normalmapName.empty() && (mtiNormalMap = this->GetInfoByName(normalmapName), mtiNormalMap != nullptr)) {
                        result.normalmap = normalmapName;
                        if (!allMips || !mtiNormalMap->streamable) {
                            result.normalmapPaths.push_back(MetroFileSystem::Paths::TexturesFolder + this->GetNameWithExt(result.normalmap, *mtiNormalMap));
                        } else {
                            this->CollectAllLevelsWithExt(result.normalmap, *mtiNormalMap, result.normalmapPaths);
                        }
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

        CharString GetNameWithExt(const CharString&, const MetroTextureInfo& mti) const {
            return mti.source_name + ((mti.streamable) ? (CharString(".") + std::to_string(mti.width)) : ".dds");
        }

        void CollectAllLevelsWithExt(const CharString& name, const MetroTextureInfo& mti, StringArray& dest) const {
            size_t width = mti.width;
            while (width >= 512) {
                dest.push_back(MetroFileSystem::Paths::TexturesFolder + name + '.' + std::to_string(width));
                width >>= 1;
            }
        }

    private:
        mutable MyArray<MetroTextureInfo>   mPool;
        mutable MyDict<HashString, size_t>  mDatabase;
        MyDict<HashString, HashString>      mAliases;
    };

} // namespace Arktika1Impl
