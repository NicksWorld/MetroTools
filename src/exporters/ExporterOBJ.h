#pragma once
#include "mycommon.h"

class MetroModelBase;

class ExporterOBJ {
public:
    ExporterOBJ();
    ~ExporterOBJ();

    void        SetExcludeCollision(const bool b);
    void        SetExporterName(const CharString& name);
    void        SetTexturesExtension(const CharString& ext);
    bool        ExportModel(const MetroModelBase& model, const fs::path& filePath) const;

private:
    bool        mExcludeCollision;
    CharString  mExporterName;
    CharString  mTexturesExtension;
};
