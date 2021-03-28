#include "renderpanel.h"

#include <QEvent>
#include <QWheelEvent>
#include <QApplication>

#include "metro/MetroModel.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"
#include "metro/MetroTexture.h"
#include "metro/MetroLightProbe.h"
#include "metro/MetroLevel.h"
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
    , mModel(nullptr)
    , mModelNode(nullptr)
    , mCurrentMotion(nullptr)
    , mAnimPlaying(false)
    //
    , mLevel(nullptr)
    , mLevelNode(nullptr)
    , mLMBDown(false)
    , mRMBDown(false)
    , mZoom(1.0f)
    , mShowWireframe(false)
    , mShowCollision(false)
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

void RenderPanel::SetModel(MetroModel* model) {
    //mCubemap = nullptr;
    //MySafeDelete(mLightProbe);

    if (mModel != model) {
        MySafeDelete(mModel);

        mModel = model;
        //mCurrentMotion = nullptr;

        mCamera->SwitchMode(u4a::Camera::Mode::Arcball);

        mScene->Clear();
        if (mModel) {
            u4a::ResourcesManager::Get().Clear();

            mModelNode = u4a::Spawner::SpawnModel(*mScene, mModel, vec3(0.0f), true);
        }

        //    this->ResetAnimation();
        this->ResetCamera();
    }
}

void RenderPanel::SetLevel(MetroLevel* level) {
    //mCubemap = nullptr;
    //MySafeDelete(mLightProbe);
    MySafeDelete(mModel);

    if (mLevel != level) {
        MySafeDelete(mLevel);

        mLevel = level;

        mCamera->SwitchMode(u4a::Camera::Mode::FirstPerson);

        mScene->Clear();
        if (mLevel) {
            u4a::ResourcesManager::Get().Clear();

            mLevelNode = u4a::Spawner::SpawnLevelGeo(*mScene, mLevel, vec3(0.0f));
        }

        this->ResetCamera();
    }
}

void RenderPanel::SetLod(const size_t lodId) {
    if (mModel && lodId >= 0 && lodId <= 2) {
        MetroModel* baseModel = mModel;

        mModel = (lodId == 0) ? baseModel : baseModel->GetLodModel(lodId - 1);

        mCurrentMotion = nullptr;

        mCamera->SwitchMode(u4a::Camera::Mode::Arcball);

        //this->ResetAnimation();

        mModel = baseModel;
    }
}

void RenderPanel::SwitchMotion(const size_t idx) {
    if (mModel && mModel->IsAnimated()) {
        if (mModelNode) {
            u4a::Animator* animator = scast<u4a::ModelNode*>(mModelNode)->GetAnimator();
            if (animator) {
                const u4a::Animator::AnimState oldState = animator->GetAnimState();
                animator->Stop();
                animator->SetMotion(mModel->GetMotion(idx));
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
        u4a::Animator* animator = scast<u4a::ModelNode*>(mModelNode)->GetAnimator();
        if (animator) {
            result = (animator->GetAnimState() == u4a::Animator::AnimState::Playing);
        }
    }
    return result;
}

void RenderPanel::PlayAnim(const bool play) {
    if (mModel && mModel->IsAnimated() && mModelNode) {
        u4a::Animator* animator = scast<u4a::ModelNode*>(mModelNode)->GetAnimator();
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
    } else if (mLevel) {
        const float nearZ = 0.1f;
        const float farZ = 1000.0f;

        //vec3 ttt(700.417236, 20.889645, -559.636414);
        //mCamera->LookAt(ttt - vec3(3.0f), ttt);
        mCamera->LookAt(vec3(5.0f), vec3(0.0f));

        this->UpdateCamera();
        mCamera->SetViewPlanes(nearZ, farZ);
    }
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
    return Q_NULLPTR;
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
            const float t = scast<u4a::ModelNode*>(mModelNode)->GetBSphere().radius * 0.5f;

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

    if (mLevelNode && mLMBDown) {
        const bool isWDown = (::GetAsyncKeyState(0x57) & 0x8000) != 0;
        const bool isSDown = (::GetAsyncKeyState(0x53) & 0x8000) != 0;
        const bool isADown = (::GetAsyncKeyState(0x41) & 0x8000) != 0;
        const bool isDDown = (::GetAsyncKeyState(0x44) & 0x8000) != 0;

        const bool isLShiftDown = (::GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
        const bool isRShiftDown = (::GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
        const bool isShiftDown = isLShiftDown || isRShiftDown;

        const vec3& axisZ = mCamera->GetDirection();
        const vec3& axisX = mCamera->GetSide();
        vec3 pos = mCamera->GetPosition();

        const float speed = isShiftDown ? 50.0f : 10.0f;

        if (isWDown) {
            pos += axisZ * speed * dtSeconds;
        }
        if (isSDown) {
            pos -= axisZ * speed * dtSeconds;
        }
        if (isDDown) {
            pos += axisX * speed * dtSeconds;
        }
        if (isADown) {
            pos -= axisX * speed * dtSeconds;
        }

        mCamera->SetPosition(pos);
    }

    if (this->isVisible() && mScene) {
        mScene->Update(dtSeconds);
    }

    this->Render();
}
