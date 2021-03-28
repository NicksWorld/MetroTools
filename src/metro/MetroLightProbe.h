#pragma once
#include "mycommon.h"
#include "mymath.h"

class MetroTexture;

class MetroLightProbe {
public:
    MetroLightProbe();
    ~MetroLightProbe();

    bool            LoadFromPath(const CharString& path);
    bool            LoadFromData(MemStream& stream);

    MetroTexture*   GetTexture0() const;
    MetroTexture*   GetTexture1() const;
    const mat4&     GetMatrix() const;

private:
    MetroTexture*   mTexture0;
    MetroTexture*   mTexture1;
    mat4            mMatrix;
};
