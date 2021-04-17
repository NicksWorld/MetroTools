#include "RenderPanel.h"

#include "engine/Renderer.h"
#include "engine/Swapchain.h"
#include "engine/Scene.h"
#include "engine/Camera.h"
#include "engine/Spawner.h"
#include "engine/ResourcesManager.h"

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"


#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


RenderPanel::RenderPanel()
    : mWidth(0)
    , mHeight(0)
    , mModelNode(nullptr)
    , mZoom(1.0f)
    , mLMBDown(false)
    , mRMBDown(false)
    , mLastLMPos(0.0f)
    , mDrawBounds(false)
    , mDrawSubBounds(false)
    , mBoundsType(DebugBoundsType::Box)
    , mShowBones(false)
    , mShowBonesLinks(false)
    , mShowBonesNames(false)
    // text drawing
    , mD2DFactory(nullptr)
    , mD2DRT(nullptr)
    , mDWFactory(nullptr)
    , mDWTextFmt(nullptr)
    , mD2DDPIScale(1.0f)
{
}
RenderPanel::~RenderPanel() {
}

bool RenderPanel::Initialize(void* hwnd, const size_t width, const size_t height) {
    ID3D11Device* device = u4a::Renderer::Get().GetDevice();

    mSwapchain = MakeStrongPtr<u4a::Swapchain>();
    const bool result = mSwapchain->Initialize(device, hwnd);
    if (!result) {
        mSwapchain = nullptr;
    } else {
        D2D1_FACTORY_OPTIONS d2dOptions = {
#ifdef _DEBUG
            D2D1_DEBUG_LEVEL_WARNING
#else
            D2D1_DEBUG_LEVEL_NONE
#endif
        };

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &d2dOptions, (void**)&mD2DFactory);
        if (FAILED(hr)) {
            return false;
        }
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&mDWFactory);
        if (FAILED(hr)) {
            return false;
        }
        hr = mDWFactory->CreateTextFormat(L"Courier New",
                                          nullptr,
                                          DWRITE_FONT_WEIGHT_REGULAR,
                                          DWRITE_FONT_STYLE_NORMAL,
                                          DWRITE_FONT_STRETCH_NORMAL,
                                          10.0f,
                                          L"en-us",
                                          &mDWTextFmt);
        if (FAILED(hr)) {
            return false;
        }

        this->CreateD2DResources();

        mScene = MakeStrongPtr<u4a::Scene>();
        mScene->Initialize();

        mCamera = MakeStrongPtr<u4a::Camera>();
        mCamera->SetViewport(ivec4(0, 0, scast<int>(width), scast<int>(height)));
        mCamera->SetViewPlanes(0.0f, 1.0f);
        mCamera->LookAt(vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
        mScene->SetCamera(mCamera.get());

        mWidth = width;
        mHeight = height;
    }

    return result;
}

void RenderPanel::OnResize(const size_t width, const size_t height) {
    if (mWidth != width || mHeight != height) {
        this->ReleaseD2DResources();

        if (mSwapchain) {
            mSwapchain->Resize(width, height);
        }
        if (mCamera) {
            mCamera->SetViewport(ivec4(0, 0, scast<int>(width), scast<int>(height)));
        }

        mWidth = width;
        mHeight = height;

        this->CreateD2DResources();
    }
}

void RenderPanel::OnMouseButton(const bool left, const bool right, const float x, const float y) {
    if (mLMBDown != left) {
        mLMBDown = left;

        if (left) {
            mLastLMPos = vec2(x, y);
        }
    }
}

void RenderPanel::OnMouseMove(const float x, const float y) {
    static const float kCameraRotateSpeed = 0.2f;
    static const float kModelMoveSpeed = 0.01f;

    if (mLMBDown) {
        vec2 pos(x, y);

        vec2 delta = pos - mLastLMPos;
        mLastLMPos = pos;

        mCamera->Rotate(-delta.x * kCameraRotateSpeed, -delta.y * kCameraRotateSpeed);
    }
}

void RenderPanel::OnMouseWheel(const float delta) {
    if (delta > 0) {
        mZoom = std::min<float>(mZoom + 0.1f, 5.0f);
    } else if (delta < 0) {
        mZoom = std::max<float>(mZoom - 0.1f, 0.1f);
    }

    this->UpdateCamera();
}

void RenderPanel::Draw() {
    mScene->Update(0.0f);

    u4a::Renderer& renderer = u4a::Renderer::Get();

    renderer.StartFrame(*mSwapchain);
    renderer.DrawScene(*mScene);

    bool showBonesNames = false;
    if (mModel) {
        StringArray bonesNames;
        MyArray<vec2> bonesNamesPos;

        const vec3& modelPos = mModelNode->GetPosition();

        renderer.BeginDebugDraw();

        if (mDrawBounds) {
            MyArray<MetroModelGeomData> gds;
            mModel->CollectGeomData(gds);

            const color4f color(1.0f, 0.85f, 0.0f, 1.0f);

            if (mDrawSubBounds) {
                for (const auto& gd : gds) {
                    if (mBoundsType == DebugBoundsType::Box) {
                        AABBox bbox = gd.bbox;
                        bbox.minimum += modelPos;
                        bbox.maximum += modelPos;
                        renderer.DebugDrawBBox(bbox, color);
                    } else {
                        BSphere bsphere = gd.bsphere;
                        bsphere.center += modelPos;
                        renderer.DebugDrawBSphere(bsphere, color);
                    }
                }
            } else {
                if (mBoundsType == DebugBoundsType::Box) {
                    AABBox bbox = mModel->GetBBox();
                    bbox.minimum += modelPos;
                    bbox.maximum += modelPos;
                    renderer.DebugDrawBBox(bbox, color);
                } else {
                    BSphere bsphere = mModel->GetBSphere();
                    bsphere.center += modelPos;
                    renderer.DebugDrawBSphere(bsphere, color);
                }
            }
        }

        if (mShowBones) {
            RefPtr<MetroSkeleton> skeleton;
            if (mModel->GetModelType() == MetroModelType::Skeleton || mModel->GetModelType() == MetroModelType::Skeleton2) {
                skeleton = SCastRefPtr<MetroModelSkeleton>(mModel)->GetSkeleton();
            }

            if (skeleton) {
                showBonesNames = mShowBonesNames;
                mat4 view = mCamera->GetTransform();
                mat4 proj = mCamera->GetProjection();
                mat4 viewProj = proj * view;

                const color4f colorPoints(1.0f, 0.85f, 0.0f, 1.0f);
                const color4f colorTets(1.0f, 0.12f, 0.95f, 1.0f);

                const float r = 0.02f;

                const size_t numBones = skeleton->GetNumBones();
                for (size_t i = 0; i < numBones; ++i) {
                    const size_t pid = skeleton->GetBoneParentIdx(i);
                    const mat4& m = skeleton->GetBoneFullTransform(i);
                    const vec3 a = vec3(m[3]) + modelPos;

                    if (showBonesNames) {
                        vec4 ap = viewProj * vec4(a, 1.0f);
                        vec2 ap2 = vec2(ap.x / ap.w, ap.y / ap.w) * vec2(0.5f, -0.5f) + 0.5f;

                        ap2.x *= mWidth;
                        ap2.y *= mHeight;

                        bonesNames.push_back(skeleton->GetBoneName(i));
                        bonesNamesPos.push_back(ap2);
                    }


                    renderer.DebugDrawBSphere({ a, r }, colorPoints);

                    if (mShowBonesLinks && pid != kInvalidValue) {
                        const mat4& mp = skeleton->GetBoneFullTransform(pid);
                        const vec3 b = vec3(mp[3]) + modelPos;

                        renderer.DebugDrawTetrahedron(b, a, r, colorTets);
                    }
                }
            }
        }

        renderer.EndDebugDraw();

        if (showBonesNames) {
            ID2D1SolidColorBrush* brush = nullptr;
            const D2D1_COLOR_F color = { 0.231f, 0.0f, 1.0f, 1.0f };
            mD2DRT->CreateSolidColorBrush(color, &brush);

            mD2DRT->BeginDraw();
            for (size_t i = 0; i < bonesNames.size(); ++i) {
                const vec2& pos = bonesNamesPos[i];
                this->DrawText(pos.x, pos.y, bonesNames[i], brush);
            }
            mD2DRT->EndDraw();

            MySafeRelease(brush);
        }
    }

    mSwapchain->Present();
}

void RenderPanel::SetModel(const RefPtr<MetroModelBase>& model) {
    mModel = model;

    if (mModel) {
        mCamera->SwitchMode(u4a::Camera::Mode::Arcball);

        mScene->Clear();
        if (mModel) {
            u4a::ResourcesManager::Get().Clear();

            const BSphere& bsphere = mModel->GetBSphere();

            mModelNode = u4a::Spawner::SpawnModel(*mScene, mModel.get(), -bsphere.center, false);

            if (mModelNode) {
                mZoom = 1.5f;

                const float r = bsphere.radius;
                mCamera->LookAt(vec3(r * 0.3f, r, r), vec3(0.0f));

                const float nearZ = r * 0.01f;
                const float farZ = r * 20.0f;

                mCamera->SetViewPlanes(nearZ, farZ);

                this->UpdateCamera();
            }
        }
    }
}

RefPtr<MetroModelBase> RenderPanel::GetModel() const {
    return mModel;
}

void RenderPanel::UpdateModelProps() {
    if (mModel) {
        mScene->Clear();

        const BSphere& bsphere = mModel->GetBSphere();
        mModelNode = u4a::Spawner::SpawnModel(*mScene, mModel.get(), -bsphere.center, false);
    }
}

void RenderPanel::SetDebugShowBounds(const bool show) {
    mDrawBounds = show;
}

void RenderPanel::SetDebugShowSubmodelsBounds(const bool show) {
    mDrawSubBounds = show;
}

void RenderPanel::SetDebugBoundsType(const RenderPanel::DebugBoundsType dbt) {
    mBoundsType = dbt;
}

void RenderPanel::SetDebugSkeletonShowBones(const bool show) {
    mShowBones = show;
}

void RenderPanel::SetDebugSkeletonShowBonesLinks(const bool show) {
    mShowBonesLinks = show;
}

void RenderPanel::SetDebugSkeletonShowBonesNames(const bool show) {
    mShowBonesNames = show;
}


void RenderPanel::CreateD2DResources() {
    FLOAT dpiX, dpiY;
    mD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

    mD2DDPIScale = vec2(96.0f / dpiX, 96.0f / dpiY);

    D2D1_RENDER_TARGET_PROPERTIES d2dRTProps =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpiX,
            dpiY
        );

    ID3D11Texture2D* backbuffer = mSwapchain->GetBackbuffer();
    IDXGISurface* dxgiBackbuffer;
    HRESULT hr = backbuffer->QueryInterface(&dxgiBackbuffer);
    hr = mD2DFactory->CreateDxgiSurfaceRenderTarget(dxgiBackbuffer, &d2dRTProps, &mD2DRT);
    MySafeRelease(dxgiBackbuffer);
}

void RenderPanel::ReleaseD2DResources() {
    MySafeRelease(mD2DRT);
}

void RenderPanel::UpdateCamera() {
    if (mModel) {
        const float r = mModel->GetBSphere().radius * mZoom;
        vec3 invDir = -mCamera->GetDirection();
        mCamera->InitArcball(invDir * r, vec3(0.0f));
    }
}

void RenderPanel::DrawText(const float x, const float y, const CharString& text, ID2D1Brush* brush) {
    WideString wide = StrUtf8ToWide(text);

    IDWriteTextLayout* txtLayout = nullptr;
    if (SUCCEEDED(mDWFactory->CreateTextLayout(wide.data(), scast<UINT32>(wide.length()), mDWTextFmt, 800.0f, 800.0f, &txtLayout))) {
        mD2DRT->DrawTextLayout({ x * mD2DDPIScale.x, y * mD2DDPIScale.y }, txtLayout, brush);
        MySafeRelease(txtLayout);
    }
}

