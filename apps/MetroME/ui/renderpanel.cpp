#include "renderpanel.h"

#include <QEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QWindow>

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroTexture.h"
#include "metro/MetroContext.h"

#include "engine/Renderer.h"
#include "engine/Spawner.h"
#include "engine/ResourcesManager.h"
#include "engine/scenenodes/ModelNode.h"
#include "engine/scenenodes/DebugGeoNode.h"
#include "engine/Animator.h"



RenderPanel::RenderPanel(QWidget* parent)
    : QWidget(parent)
    , mHWnd(rcast<HWND>(this->winId()))
    , mSwapchain(nullptr)
    , mScene(nullptr)
    , mCamera(nullptr)
    , mCameraOffset(0.0f)
    // model viewer stuff
    , mModel{}
    , mModelNode(nullptr)
    , mCurrentMotion(nullptr)
    , mAnimPlaying(false)
    //
    , mDebugGeoNode(nullptr)
    //
    , mLMBDown(false)
    , mRMBDown(false)
    , mZoom(1.0f)
    , mShowModel(true)
    , mShowWireframe(false)
    , mShowCollision(false)
    // debug
    , mDrawBounds(false)
    , mDrawSubBounds(false)
    , mBoundsType(DebugBoundsType::Box)
    , mShowBones(false)
    , mShowBonesLinks(false)
    , mShowBonesNames(false)
    , mShowPhysics(false)
    // text drawing
    , mD2DFactory(nullptr)
    , mD2DRT(nullptr)
    , mDWFactory(nullptr)
    , mDWTextFmt(nullptr)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setAttribute(Qt::WA_NativeWindow);

    // Setting these attributes to our widget and returning null on paintEngine event
    // tells Qt that we'll handle all drawing and updating the widget ourselves.
    this->setAttribute(Qt::WA_PaintOnScreen);
    this->setAttribute(Qt::WA_NoSystemBackground);
}
RenderPanel::~RenderPanel() {
    this->ReleaseD2DResources();

    MySafeRelease(mDWTextFmt);
    MySafeRelease(mDWFactory);
    MySafeRelease(mD2DFactory);

    MySafeDelete(mSwapchain);
    MySafeDelete(mCamera);
    MySafeDelete(mScene);
}

bool RenderPanel::Initialize() {
    ID3D11Device* device = u4a::Renderer::Get().GetDevice();

    mSwapchain = new u4a::Swapchain();
    bool result = mSwapchain->Initialize(device, mHWnd);
    if (!result) {
        MySafeDelete(mSwapchain);
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

        mScene = new u4a::Scene();
        mScene->Initialize();

        mCamera = new u4a::Camera();
        mCamera->SetViewport(ivec4(0, 0, this->width(), this->height()));
        mCamera->SetViewPlanes(0.0f, 1.0f);
        mCamera->LookAt(vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
        mScene->SetCamera(mCamera);

        connect(&mTimer, &QTimer::timeout, this, &RenderPanel::OnFrame);
        mTimer.start(16);
    }

    return result;
}

void RenderPanel::SetModel(const RefPtr<MetroModelBase>& model) {
    if (mModel != model) {
        mModel = model;

        mCamera->SwitchMode(u4a::Camera::Mode::Arcball);

        mScene->Clear();
        if (mModel) {
            u4a::ResourcesManager::Get().Clear();

            const vec3& center = mModel->GetBBox().Center();
            mModelNode = scast<u4a::ModelNode*>(u4a::Spawner::SpawnModel(*mScene, mModel.get(), -center, true));
        }

        mDebugGeo = nullptr;
        mDebugGeoNode = nullptr;

        this->ResetCamera();
    }
}

const RefPtr<MetroModelBase>& RenderPanel::GetModel() const {
    return mModel;
}

void RenderPanel::UpdateModelProps() {
    if (mModel) {
        const vec3& center = mModel->GetBBox().Center();
        vec3 pos = (mModelNode != nullptr) ? mModelNode->GetPosition() : -center;

        mScene->Clear();
        mModelNode = scast<u4a::ModelNode*>(u4a::Spawner::SpawnModel(*mScene, mModel.get(), pos, true));
    }
}

void RenderPanel::SetDebugGeo(const RefPtr<u4a::DebugGeo>& debugGeo) {
    mDebugGeo = debugGeo;
    if (mDebugGeo) {
        vec3 pos = (mModelNode == nullptr) ? vec3(0.0f) : mModelNode->GetPosition();

        mDebugGeoNode = scast<u4a::DebugGeoNode*>(u4a::Spawner::SpawnDebugGeoNode(*mScene, mDebugGeo.get(), pos));
    }
}

void RenderPanel::SetLod(const size_t lodId) {
    if (mModelNode && mModel) {
        mModelNode->SetLod(lodId);
    }
}

void RenderPanel::SwitchMotion(const size_t idx) {
    if (mModel && mModel->IsSkeleton()) {
        RefPtr<MetroModelSkeleton> skelMdl = SCastRefPtr<MetroModelSkeleton>(mModel);
        RefPtr<MetroSkeleton> skeleton = skelMdl->GetSkeleton();

        if (skeleton && mModelNode) {
            u4a::Animator* animator = scast<u4a::ModelNode*>(mModelNode)->GetAnimator();
            if (animator) {
                const u4a::Animator::AnimState oldState = animator->GetAnimState();
                animator->Stop();
                animator->SetMotion(skeleton->GetMotion(idx));
                if (oldState == u4a::Animator::AnimState::Playing) {
                    animator->Play();
                }
            }
        }
    }
}

bool RenderPanel::IsPlayingAnim() const {
    bool result = false;
    if (mModelNode) {
        u4a::Animator* animator = mModelNode->GetAnimator();
        if (animator) {
            result = (animator->GetAnimState() == u4a::Animator::AnimState::Playing);
        }
    }
    return result;
}

void RenderPanel::PlayAnim(const bool play) {
    if (mModel && mModel->IsSkeleton() && mModelNode) {
        u4a::Animator* animator = mModelNode->GetAnimator();
        if (animator) {
            if (play) {
                animator->Play();
            } else {
                animator->Stop();
            }
        }
    }
}



void RenderPanel::ResetCamera() {
    mZoom = 1.5f;
    mCameraOffset = vec3(0.0f);

    if (mModel) {
        const float r = mModel->GetBSphere().radius;
        const float nearZ = r * 0.05f;
        const float farZ = r * 100.0f;

        mCamera->LookAt(vec3(r * 0.3f, r, r), vec3(0.0f));

        this->UpdateCamera();
        mCamera->SetViewPlanes(nearZ, farZ);

        if (mModelNode) {
            mModelNode->SetPosition(vec3(0.0f));
        }
    }
}

void RenderPanel::SetShowModel(const bool show) {
    mShowModel = show;
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

bool RenderPanel::IsShowingSkeletonShowBones() const {
    return mShowBones;
}

void RenderPanel::SetDebugSkeletonShowBonesLinks(const bool show) {
    mShowBonesLinks = show;
}

void RenderPanel::SetDebugSkeletonShowBonesNames(const bool show) {
    mShowBonesNames = show;
}

void RenderPanel::SetDebugShowPhysics(const bool show) {
    mShowPhysics = show;
}



void RenderPanel::CreateD2DResources() {
    float dpi = scast<float>(::GetDpiForWindow(mHWnd));
    if (dpi == 0.0f) {
        dpi = 96.0f;
    }

    D2D1_RENDER_TARGET_PROPERTIES d2dRTProps =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpi,
            dpi
        );

    ID3D11Texture2D* backbuffer = mSwapchain->GetBackbuffer();
    IDXGISurface* dxgiBackbuffer = nullptr;
    HRESULT hr = backbuffer->QueryInterface(&dxgiBackbuffer);
    if (SUCCEEDED(hr)) {
        hr = mD2DFactory->CreateDxgiSurfaceRenderTarget(dxgiBackbuffer, &d2dRTProps, &mD2DRT);
        MySafeRelease(dxgiBackbuffer);
    }
}

void RenderPanel::ReleaseD2DResources() {
    MySafeRelease(mD2DRT);
}

void RenderPanel::UpdateCamera() {
    if (mModel) {
        const float r = mModel->GetBSphere().radius * mZoom;
        vec3 invDir = -mCamera->GetDirection();
        mCamera->InitArcball(invDir * r, vec3(0.0f));
        mCamera->SetPosition(mCamera->GetPosition() + mCameraOffset);
    } else {
        vec3 invDir = -mCamera->GetDirection();
        mCamera->InitArcball(invDir * mZoom, vec3(0.0f));
    }
}

void RenderPanel::Render() {
    if (!this->isVisible()) {
        return;
    }

    if (mSwapchain) {
        u4a::Renderer& renderer = u4a::Renderer::Get();
        renderer.StartFrame(*mSwapchain);

        size_t renderFlags = u4a::Renderer::DF_None;
        if (!mShowModel) {
            renderFlags |= u4a::Renderer::DF_SkipModels;
        }
        if (!mShowPhysics) {
            renderFlags |= u4a::Renderer::DF_SkipDebugGeo;
        }

        renderer.DrawScene(*mScene, renderFlags);

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
                if (mModel->IsSkeleton()) {
                    skeleton = SCastRefPtr<MetroModelSkeleton>(mModel)->GetSkeleton();
                }

                if (skeleton) {
                    showBonesNames = mShowBonesNames;
                    mat4 view = mCamera->GetTransform();
                    mat4 proj = mCamera->GetProjection();
                    mat4 viewProj = proj * view;

                    const color4f colorPoints(1.0f, 0.85f, 0.0f, 1.0f);
                    const color4f colorPointsLoc(0.0f, 0.85f, 0.1f, 1.0f);
                    const color4f colorTets(1.0f, 0.12f, 0.95f, 1.0f);
                    const color4f colorTetsLoc(0.1f, 0.1f, 1.0f, 1.0f);

                    const float r = 0.02f;

                    const size_t numBones = skeleton->GetNumBones();
                    for (size_t i = 0; i < numBones; ++i) {
                        const size_t pid = skeleton->GetBoneParentIdx(i);
                        const mat4& m = skeleton->GetBoneFullTransform(i);
                        const vec3 a = vec3(m[3]) + modelPos;

                        if (showBonesNames) {
                            vec4 ap = viewProj * vec4(a, 1.0f);
                            vec2 ap2 = vec2(ap.x / ap.w, ap.y / ap.w) * vec2(0.5f, -0.5f) + 0.5f;

                            ap2.x *= scast<float>(this->width());
                            ap2.y *= scast<float>(this->height());

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

                    const size_t numLocators = skeleton->GetNumLocators();
                    for (size_t i = 0; i < numLocators; ++i) {
                        const size_t pid = skeleton->GetBoneParentIdx(i + numBones);
                        const mat4& m = skeleton->GetBoneFullTransform(i + numBones);
                        const vec3 a = vec3(m[3]) + modelPos;

                        if (showBonesNames) {
                            vec4 ap = viewProj * vec4(a, 1.0f);
                            vec2 ap2 = vec2(ap.x / ap.w, ap.y / ap.w) * vec2(0.5f, -0.5f) + 0.5f;

                            ap2.x *= scast<float>(this->width());
                            ap2.y *= scast<float>(this->height());

                            bonesNames.push_back(skeleton->GetBoneName(i + numBones));
                            bonesNamesPos.push_back(ap2);
                        }

                        renderer.DebugDrawBSphere({ a, r }, colorPointsLoc);

                        if (mShowBonesLinks && pid != kInvalidValue) {
                            const mat4& mp = skeleton->GetBoneFullTransform(pid);
                            const vec3 b = vec3(mp[3]) + modelPos;

                            renderer.DebugDrawTetrahedron(b, a, r, colorTetsLoc);
                        }
                    }
                }
            }

            renderer.EndDebugDraw();

            if (showBonesNames) {
                ID2D1SolidColorBrush* brush = nullptr;
                ID2D1SolidColorBrush* brushShadow = nullptr;
                const D2D1_COLOR_F color = { 0.231f, 0.0f, 1.0f, 1.0f };
                const D2D1_COLOR_F colorShadow = { 0.0f, 0.0f, 0.0f, 1.0f };
                mD2DRT->CreateSolidColorBrush(color, &brush);
                mD2DRT->CreateSolidColorBrush(colorShadow, &brushShadow);

                mD2DRT->BeginDraw();
                for (size_t i = 0; i < bonesNames.size(); ++i) {
                    const vec2& pos = bonesNamesPos[i];
                    this->DrawText2D(pos.x, pos.y, bonesNames[i], brush);
                    this->DrawText2D(pos.x + 0.5f, pos.y + 0.5f, bonesNames[i], brushShadow);
                }
                mD2DRT->EndDraw();

                MySafeRelease(brush);
                MySafeRelease(brushShadow);
            }
        }

        mSwapchain->Present();
    }
}

void RenderPanel::DrawText2D(const float x, const float y, const CharString& text, ID2D1Brush* brush) {
    WideString wide = StrUtf8ToWide(text);

    IDWriteTextLayout* txtLayout = nullptr;
    if (SUCCEEDED(mDWFactory->CreateTextLayout(wide.data(), scast<UINT32>(wide.length()), mDWTextFmt, 800.0f, 800.0f, &txtLayout))) {
        mD2DRT->DrawTextLayout({ x, y }, txtLayout, brush);
        MySafeRelease(txtLayout);
    }
}

bool RenderPanel::event(QEvent* event) {
    switch (event->type()) {
        // Workaround for https://bugreports.qt.io/browse/QTBUG-42183 to get key strokes.
        // To make sure that we always have focus on the widget when we enter the rect area.
        case QEvent::Enter:
        case QEvent::FocusIn:
        case QEvent::FocusAboutToChange: {
            if (::GetFocus() != mHWnd) {
                QWidget* nativeParent = this;
                for (;;) {
                    if (nativeParent->isWindow()) {
                        break;
                    }

                    QWidget* parent = nativeParent->nativeParentWidget();
                    if (!parent) {
                        break;
                    }

                    nativeParent = parent;
                }

                if (nativeParent && nativeParent != this && ::GetFocus() == rcast<HWND>(nativeParent->winId())) {
                    ::SetFocus(mHWnd);
                }
            }
        } break;

        default:
            break;
    }

    return QWidget::event(event);
}

void RenderPanel::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

QPaintEngine* RenderPanel::paintEngine() const {
    return nullptr;
}

void RenderPanel::paintEvent(QPaintEvent* event) {
}

static QScreen* GetActiveScreen(QWidget* pWidget) {
    QScreen* pActive = nullptr;

    while (pWidget) {
        auto w = pWidget->windowHandle();
        if (w != nullptr) {
            pActive = w->screen();
            break;
        } else {
            pWidget = pWidget->parentWidget();
        }
    }

    return pActive;
}

void RenderPanel::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    qreal pixelRatio = qApp->devicePixelRatio();
    QScreen* screen = GetActiveScreen(this);
    if (screen) {
        pixelRatio = screen->devicePixelRatio();
    }

    const int realWidth = this->width() * pixelRatio;
    const int realHeight = this->height() * pixelRatio;

    this->ReleaseD2DResources();

    if (mSwapchain) {
        mSwapchain->Resize(scast<size_t>(realWidth), scast<size_t>(realHeight));
    }
    if (mCamera) {
        mCamera->SetViewport(ivec4(0, 0, realWidth, realHeight));
    }

    this->CreateD2DResources();
}

void RenderPanel::mouseMoveEvent(QMouseEvent* event) {
    static const float kCameraRotateSpeed = 0.2f;
    static const float kModelMoveSpeed = 0.01f;

    QPoint mp = event->pos();

    if (mLMBDown) {
        const float deltaX = scast<float>(mp.x() - mLastLMPos.x());
        const float deltaY = scast<float>(mp.y() - mLastLMPos.y());

        mLastLMPos = mp;

        mCamera->Rotate(deltaX * kCameraRotateSpeed, deltaY * kCameraRotateSpeed);
    } else if (mRMBDown) {
        const float deltaX = scast<float>(mp.x() - mLastRMPos.x());
        const float deltaY = scast<float>(mp.y() - mLastRMPos.y());

        mLastRMPos = mp;

        if (mModelNode) {
            const float t = mModelNode->GetBSphere().radius * 0.5f;

            vec3 tx = mCamera->GetSide() * deltaX * kModelMoveSpeed * t;
            vec3 ty = mCamera->GetUp() * -deltaY * kModelMoveSpeed * t;

            mCameraOffset -= (tx + ty);
            this->UpdateCamera();
        }
    }
}

void RenderPanel::mousePressEvent(QMouseEvent* event) {
    QPoint mp = event->pos();

    if (event->button() == Qt::LeftButton) {
        mLastLMPos = mp;
        mLMBDown = true;
    } else if (event->button() == Qt::RightButton) {
        mLastRMPos = mp;
        mRMBDown = true;
    }

    QWidget::mousePressEvent(event);
}

void RenderPanel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mLMBDown = false;
    } else if (event->button() == Qt::RightButton) {
        mRMBDown = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void RenderPanel::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();

    QWidget::wheelEvent(event);

    if (delta > 0) {
        mZoom = std::min<float>(mZoom + 0.1f, 5.0f);
    } else if (delta < 0) {
        mZoom = std::max<float>(mZoom - 0.1f, 0.1f);
    }

    this->UpdateCamera();
}

bool RenderPanel::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    MSG* pMsg = rcast<MSG*>(message);

    // Process wheel events using Qt's event-system.
    if (pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL) {
        return false;
    }
    return false;
#else
    return QWidget::nativeEvent(eventType, message, result);
#endif
}



void RenderPanel::OnFrame() {
    static double sLastTimeMS = -1.0;

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point::duration epoch = tp.time_since_epoch();

    const double currentTimeMS = scast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count());

    if (sLastTimeMS < 0.0) {
        sLastTimeMS = currentTimeMS;
    }

    const float dtSeconds = scast<float>((currentTimeMS - sLastTimeMS) * 0.001);
    sLastTimeMS = currentTimeMS;

    if (this->isVisible() && mScene) {
        mScene->Update(dtSeconds);
    }

    this->Render();
}
