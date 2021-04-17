#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "d2d1.h"
#include "dwrite.h"

namespace u4a {
    class Swapchain;
    class Scene;
    class SceneNode;
    class Camera;
} // namespace u4a

class MetroModelBase;


class RenderPanel {
public:
    enum class DebugBoundsType {
        Box,
        Sphere
    };

public:
    RenderPanel();
    ~RenderPanel();

    bool                        Initialize(void* hwnd, const size_t width, const size_t height);

    void                        OnResize(const size_t width, const size_t height);
    void                        OnMouseButton(const bool left, const bool right, const float x, const float y);
    void                        OnMouseMove(const float x, const float y);
    void                        OnMouseWheel(const float delta);

    void                        Draw();

    void                        SetModel(const RefPtr<MetroModelBase>& model);
    RefPtr<MetroModelBase>      GetModel() const;
    void                        UpdateModelProps();

    void                        SetDebugShowBounds(const bool show);
    void                        SetDebugShowSubmodelsBounds(const bool show);
    void                        SetDebugBoundsType(const DebugBoundsType dbt);
    void                        SetDebugSkeletonShowBones(const bool show);
    void                        SetDebugSkeletonShowBonesLinks(const bool show);
    void                        SetDebugSkeletonShowBonesNames(const bool show);

private:
    void                        CreateD2DResources();
    void                        ReleaseD2DResources();
    void                        UpdateCamera();
    void                        DrawText(const float x, const float y, const CharString& text, ID2D1Brush* brush);

private:
    StrongPtr<u4a::Swapchain>   mSwapchain;
    StrongPtr<u4a::Scene>       mScene;
    StrongPtr<u4a::Camera>      mCamera;
    size_t                      mWidth;
    size_t                      mHeight;

    RefPtr<MetroModelBase>      mModel;
    u4a::SceneNode*             mModelNode;

    float                       mZoom;
    bool                        mLMBDown;
    bool                        mRMBDown;
    vec2                        mLastLMPos;

    // debug
    bool                        mDrawBounds;
    bool                        mDrawSubBounds;
    DebugBoundsType             mBoundsType;
    bool                        mShowBones;
    bool                        mShowBonesLinks;
    bool                        mShowBonesNames;

    // text drawing
    ID2D1Factory*               mD2DFactory;
    ID2D1RenderTarget*          mD2DRT;
    IDWriteFactory*             mDWFactory;
    IDWriteTextFormat*          mDWTextFmt;
    vec2                        mD2DDPIScale;
};
