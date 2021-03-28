#pragma once
#include "mycommon.h"
#include "mymath.h"

namespace u4a {

class Camera {
public:
    enum class Mode {
        FirstPerson,
        Arcball
    };

public:
    Camera();
    ~Camera();

    void        SetViewport(const ivec4& viewport);
    void        SetFovY(const float fovy);
    void        SetViewPlanes(const float nearZ, const float farZ);
    void        SetZoom(const float zoom, const bool reset = false);
    void        SetPosition(const vec3& pos);

    void        SwitchMode(Mode mode);

    void        InitArcball(const vec3& pos, const vec3& target);
    void        LookAt(const vec3& pos, const vec3& target);
    void        Move(const float side, const float direction);
    void        Rotate(const float angleX, const float angleY);

    float       GetNearPlane() const;
    float       GetFarPlane() const;
    float       GetAspect() const;
    float       GetFovY() const;
    float       GetZoom() const;

    const mat4& GetProjection() const;
    const mat4& GetTransform() const;

    const vec3& GetPosition() const;
    const vec3& GetDirection() const;
    const vec3  GetUp() const;
    const vec3  GetSide() const;

    Ray         CalcRayFromScreenPoint(const vec2& screenPoint);

private:
    void        MakeProjection();
    void        MakeTransform();

private:
    Mode        mMode;
    ivec4       mViewport;
    float       mFovY;
    float       mNearZ;
    float       mFarZ;
    float       mZoom;
    vec3        mPosition;
    vec3        mDirection;
    quat        mRotation;
    mat4        mProjection;
    mat4        mTransform;
};

} // namespace u4a
