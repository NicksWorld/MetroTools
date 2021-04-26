#include "renderpanel.h"

#include <QEvent>
#include <QWheelEvent>
#include <QApplication>

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroTexture.h"
#include "metro/MetroContext.h"

#include "engine/Renderer.h"
#include "engine/Spawner.h"
#include "engine/ResourcesManager.h"
#include "engine/scenenodes/ModelNode.h"
#include "engine/Animator.h"


RenderPanel::RenderPanel(QWidget* parent)
    : QWidget(parent)
    , mHWnd(rcast<HWND>(this->winId()))
    , mSwapchain(nullptr)
    , mScene(nullptr)
    , mCamera(nullptr)
    // model viewer stuff
    , mModel{}
    , mModelNode(nullptr)
    , mCurrentMotion(nullptr)
    , mAnimPlaying(false)
    //
    , mLMBDown(false)
    , mRMBDown(false)
    , mZoom(1.0f)
    , mShowWireframe(false)
    , mShowCollision(false)
    // debug
    , mDrawBounds(false)
    , mDrawSubBounds(false)
    , mBoundsType(DebugBoundsType::Box)
    , mShowBones(false)
    , mShowBonesLinks(false)
    , mShowBonesNames(false)
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

}

bool RenderPanel::Initialize() {
    ID3D11Device* device = u4a::Renderer::Get().GetDevice();

    mSwapchain = new u4a::Swapchain();
    bool result = mSwapchain->Initialize(device, mHWnd);
    if (!result) {
        MySafeDelete(mSwapchain);
    } else {
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

            mModelNode = scast<u4a::ModelNode*>(u4a::Spawner::SpawnModel(*mScene, mModel.get(), vec3(0.0f), true));
        }

        this->ResetCamera();
    }
}

const RefPtr<MetroModelBase>& RenderPanel::GetModel() const {
    return mModel;
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



void RenderPanel::UpdateCamera() {
    if (mModel) {
        const float r = mModel->GetBSphere().radius * mZoom;
        vec3 invDir = -mCamera->GetDirection();
        mCamera->InitArcball(invDir * r, vec3(0.0f));
    }
}

void RenderPanel::Render() {
    if (!this->isVisible()) {
        return;
    }

    if (mSwapchain) {
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
                }
            }

            renderer.EndDebugDraw();

#if 0
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
#endif
        }

        mSwapchain->Present();
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

void RenderPanel::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    qreal pixelRatio = qApp->devicePixelRatio();
    const int realWidth = this->width() * pixelRatio;
    const int realHeight = this->height() * pixelRatio;

    if (mSwapchain) {
        mSwapchain->Resize(scast<size_t>(realWidth), scast<size_t>(realHeight));
    }
    if (mCamera) {
        mCamera->SetViewport(ivec4(0, 0, realWidth, realHeight));
    }
}

void RenderPanel::mouseMoveEvent(QMouseEvent* event) {
    static const float kCameraRotateSpeed = 0.2f;
    static const float kModelMoveSpeed = 0.01f;

    QPoint mp = event->pos();

    if (mLMBDown) {
        const float deltaX = scast<float>(mp.x() - mLastLMPos.x());
        const float deltaY = scast<float>(mp.y() - mLastLMPos.y());

        mLastLMPos = mp;

        mCamera->Rotate(-deltaX * kCameraRotateSpeed, -deltaY * kCameraRotateSpeed);
    } else if (mRMBDown) {
        const float deltaX = scast<float>(mp.x() - mLastRMPos.x());
        const float deltaY = scast<float>(mp.y() - mLastRMPos.y());

        mLastRMPos = mp;

        if (mModelNode) {
            const float t = mModelNode->GetBSphere().radius * 0.5f;

            vec3 tx = mCamera->GetSide() * deltaX * kModelMoveSpeed * t;
            vec3 ty = mCamera->GetUp() * -deltaY * kModelMoveSpeed * t;

            mModelNode->SetPosition(mModelNode->GetPosition() + tx + ty);
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