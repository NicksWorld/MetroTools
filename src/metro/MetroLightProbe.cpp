#include "MetroLightProbe.h"
#include "MetroContext.h"
#include "MetroTexture.h"

enum LightProbeChunk {
    LPC_Header      = 0,
    LPC_Texture1    = 1,
    LPC_Texture2    = 2,
    LPC_Matrix      = 3
};

MetroLightProbe::MetroLightProbe()
    : mTexture0(nullptr)
    , mTexture1(nullptr)
{
}
MetroLightProbe::~MetroLightProbe() {
    MySafeDelete(mTexture0);
    MySafeDelete(mTexture1);
}

bool MetroLightProbe::LoadFromPath(const CharString& path) {
    bool result = false;

    MemStream stream = MetroContext::Get().GetFilesystem().OpenFileFromPath(path);
    if (stream.Good()) {
        result = this->LoadFromData(stream);
    }

    return result;
}

bool MetroLightProbe::LoadFromData(MemStream& stream) {
    bool result = false;

    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case LPC_Texture1: {
                MemStream subStream = stream.Substream(chunkSize);

                mTexture0 = new MetroTexture();
                if (!mTexture0->LoadFromData(subStream, kEmptyString)) {
                    MySafeDelete(mTexture0);
                }
            } break;

            case LPC_Texture2: {
                MemStream subStream = stream.Substream(chunkSize);

                mTexture1 = new MetroTexture();
                if (!mTexture1->LoadFromData(subStream, kEmptyString)) {
                    MySafeDelete(mTexture1);
                }
            } break;

            case LPC_Matrix: {
                stream.ReadStruct(mMatrix);
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    result = (mTexture0 != nullptr);

    return result;
}

MetroTexture* MetroLightProbe::GetTexture0() const {
    return mTexture0;
}

MetroTexture* MetroLightProbe::GetTexture1() const {
    return mTexture1;
}

const mat4& MetroLightProbe::GetMatrix() const {
    return mMatrix;
}
