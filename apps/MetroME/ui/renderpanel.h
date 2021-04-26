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

namespace u4a {
    class ModelNode;
}

class RenderPanel : public QWidget {
    Q_OBJECT

public:
    enum class DebugBoundsType {
        Box,
        Sphere
    };

public:
    RenderPanel(QWidget* parent = nullptr);
    ~RenderPanel();

    bool                            Initialize();
    void                            SetModel(const RefPtr<MetroModelBase>& model);
    const RefPtr<MetroModelBase>&   GetModel() const;

    void                            SetLod(const size_t lodId);
    void                            SwitchMotion(const size_t idx);
    bool                            IsPlayingAnim() const;
    void                            PlayAnim(const bool play);

    void                            ResetCamera();

    void                            SetDebugShowBounds(const bool show);
    void                            SetDebugShowSubmodelsBounds(const bool show);
    void                            SetDebugBoundsType(const DebugBoundsType dbt);
    void                            SetDebugSkeletonShowBones(const bool show);
    void                            SetDebugSkeletonShowBonesLinks(const bool show);
    void                            SetDebugSkeletonShowBonesNames(const bool show);

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

    QTimer                          mTimer;

    bool                            mLMBDown;
    bool                            mRMBDown;
    QPoint                          mLastLMPos;
    QPoint                          mLastRMPos;
    float                           mZoom;
    bool                            mShowWireframe;
    bool                            mShowCollision;

    // debug
    bool                            mDrawBounds;
    bool                            mDrawSubBounds;
    DebugBoundsType                 mBoundsType;
    bool                            mShowBones;
    bool                            mShowBonesLinks;
    bool                            mShowBonesNames;
};
