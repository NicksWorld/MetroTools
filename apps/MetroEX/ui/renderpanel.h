#pragma once
#include <QWidget>
#include <QTimer>

#include "engine/Swapchain.h"
#include "engine/Scene.h"
#include "engine/Camera.h"

class MetroModelBase;
class MetroSkeleton;
class MetroMotion;
class MetroTexture;
class MetroLightProbe;
class MetroLevel;

namespace u4a {
    class ModelNode;
}

class RenderPanel : public QWidget {
    Q_OBJECT

    struct AnimBone {
        size_t  idx;
        size_t  parentIdx;
    };

    struct Animation {
        float       time;
        size_t      numBones;
        AnimBone    bones[256];
        mat4        bindPose[256];
        mat4        bindPoseInv[256];
    };

public:
    RenderPanel(QWidget* parent = nullptr);
    ~RenderPanel();

    bool                            Initialize();
    void                            SetModel(const RefPtr<MetroModelBase>& model);
    const RefPtr<MetroModelBase>&   GetModel() const;
    void                            SetLevel(MetroLevel* level);

    void                            SetLod(const size_t lodId);
    void                            SwitchMotion(const size_t idx);
    bool                            IsPlayingAnim() const;
    void                            PlayAnim(const bool play);

    void                            ResetCamera();

private:
    void                            UpdateCamera();
    void                            Render();

// Qt Events
private:
    bool                            event(QEvent* event) override;
    void                            showEvent(QShowEvent* event) override;
    QPaintEngine*                   paintEngine() const override;
    void                            paintEvent(QPaintEvent* event) override;
    void                            resizeEvent(QResizeEvent* event) override;
    void                            mouseMoveEvent(QMouseEvent* event) override;
    void                            mousePressEvent(QMouseEvent* event) override;
    void                            mouseReleaseEvent(QMouseEvent* event) override;
    void                            wheelEvent(QWheelEvent* event) override;
    bool                            nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private slots:
    void                            OnFrame();

private:
    HWND                            mHWnd;
    u4a::Swapchain*                 mSwapchain;
    u4a::Scene*                     mScene;
    u4a::Camera*                    mCamera;

    // model viewer stuff
    RefPtr<MetroModelBase>          mModel;
    u4a::ModelNode*                 mModelNode;
    const MetroMotion*              mCurrentMotion;
    bool                            mAnimPlaying;

    MetroLevel*                     mLevel;
    u4a::SceneNode*                 mLevelNode;

    QTimer                          mTimer;

    bool                            mLMBDown;
    bool                            mRMBDown;
    QPoint                          mLastLMPos;
    QPoint                          mLastRMPos;
    float                           mZoom;
    bool                            mShowWireframe;
    bool                            mShowCollision;
};
