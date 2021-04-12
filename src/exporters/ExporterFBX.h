#pragma once
#include "mycommon.h"
#include "metro/MetroTypes.h"

class MetroModel;
class MetroModelBase;
class MetroSkeleton;
class MetroMotion;
class MetroLevel;


class ExporterFBX {
public:
    ExporterFBX();
    ~ExporterFBX();

    void                SetExporterName(const CharString& exporterName);
    void                SetTexturesExtension(const CharString& ext);
    void                SetExcludeCollision(const bool b);
    void                SetExportMesh(const bool b);
    void                SetExportSkeleton(const bool b);
    void                SetExportAnimation(const bool b);
    void                SetExportMotionIdx(const size_t idx);

    bool                ExportModel(const MetroModelBase& model, const fs::path& filePath);
    bool                ExportLevel(const MetroLevel& level, const fs::path& filePath);

    const StringArray&  GetUsedTextures() const;
    const fs::path&     GetTexturesFolder() const;

private:
    CharString          mExporterName;
    CharString          mTexturesExtension;
    bool                mExcludeCollision;
    bool                mExportMesh;
    bool                mExportSkeleton;
    bool                mExportAnimation;
    size_t              mExportMotionIdx;

    StringArray         mUsedTextures;
    fs::path            mTexturesFolder;
};
