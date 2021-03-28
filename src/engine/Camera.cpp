#include "Camera.h"

static const vec3 sCameraUp(0.0f, 1.0f, 0.0f);

namespace u4a {

Camera::Camera()
    : mMode(Camera::Mode::FirstPerson)
    , mFovY(65.0f)
    , mNearZ(1.0f)
    , mFarZ(1000.0f)
    , mZoom(1.0f)
    , mPosition(0.0f, 0.0f, 0.0f)
    , mDirection(0.0f, 0.0f, 1.0f) {
}

Camera::~Camera() {
}

void Camera::SetViewport(const ivec4& viewport) {
    mViewport = viewport;
    this->MakeProjection();
}

void Camera::SetFovY(const float fovy) {
    mFovY = fovy;
    this->MakeProjection();
}

void Camera::SetViewPlanes(const float nearZ, const float farZ) {
    mNearZ = nearZ;
    mFarZ = farZ;
    this->MakeProjection();
}

void Camera::SetZoom(const float zoom, const bool reset) {
    if (mZoom != zoom) {
        vec3 lookAtPoint = mPosition + mDirection * mZoom;
        mPosition = lookAtPoint - (mDirection * zoom);
        mZoom = zoom;

        this->MakeTransform();
    }
}

void Camera::SetPosition(const vec3& pos) {
    mPosition = pos;
    this->MakeTransform();
}

void Camera::SwitchMode(Mode mode) {
    mMode = mode;
}

void Camera::InitArcball(const vec3& pos, const vec3& target) {
    mZoom = Length(target - pos);
    this->LookAt(pos, target);
}

void Camera::LookAt(const vec3& pos, const vec3& target) {
    mPosition = pos;
    mDirection = normalize(target - pos);

    this->MakeTransform();
}

void Camera::Move(const float side, const float direction) {
    vec3 cameraSide = normalize(cross(mDirection, sCameraUp));

    mPosition += cameraSide * side;
    mPosition += mDirection * direction;

    this->MakeTransform();
}

void Camera::Rotate(const float angleX, const float angleY) {
    if (Camera::Mode::FirstPerson == mMode) {
        vec3 side = cross(mDirection, sCameraUp);
        quat pitchQ = QuatAngleAxis(Deg2Rad(angleY), side);
        quat headingQ = QuatAngleAxis(Deg2Rad(angleX), sCameraUp);
        quat temp = normalize(pitchQ * headingQ);
        mDirection = normalize(QuatRotate(temp, mDirection));

        this->MakeTransform();
    } else {
        vec3 targetPoint = mPosition + mDirection * mZoom;
        vec3 dirToCam = -mDirection;

        vec3 side = this->GetSide();
        vec3 up = this->GetUp();

        quat pitchQ = QuatAngleAxis(Deg2Rad(angleY), side);
        quat headingQ = QuatAngleAxis(Deg2Rad(angleX), up);

        quat temp = normalize(pitchQ * headingQ);

        dirToCam = normalize(QuatRotate(temp, dirToCam));

        this->LookAt(targetPoint + dirToCam * mZoom, targetPoint);
    }
}

float Camera::GetNearPlane() const {
    return mNearZ;
}

float Camera::GetFarPlane() const {
    return mFarZ;
}

float Camera::GetAspect() const {
    const float aspect = scast<float>(mViewport.z - mViewport.x) / scast<float>(mViewport.w - mViewport.y);
    return aspect;
}

float Camera::GetFovY() const {
    return mFovY;
}

float Camera::GetZoom() const {
    return mZoom;
}

const mat4& Camera::GetProjection() const {
    return mProjection;
}

const mat4& Camera::GetTransform() const {
    return mTransform;
}

const vec3& Camera::GetPosition() const {
    return mPosition;
}

const vec3& Camera::GetDirection() const {
    return mDirection;
}

const vec3 Camera::GetUp() const {
    return vec3(mTransform[0][1], mTransform[1][1], mTransform[2][1]);
}

const vec3 Camera::GetSide() const {
    return vec3(mTransform[0][0], mTransform[1][0], mTransform[2][0]);
}

Ray Camera::CalcRayFromScreenPoint(const vec2& screenPoint) {
    Ray result;
    result.origin = this->GetPosition();

    vec3 u = this->GetSide();
    vec3 v = this->GetUp();

    const float planeWidth = std::tanf(Deg2Rad(mFovY) * 0.5f);
    const float aspect = this->GetAspect();

    u *= (planeWidth * aspect);
    v *= planeWidth;

    const float posXNorm = (screenPoint.x / scast<float>(mViewport.z - mViewport.x - 1)) * 2.0f - 1.0f;
    const float posYNorm = (screenPoint.y / scast<float>(mViewport.w - mViewport.y - 1)) * 2.0f - 1.0f;

    result.direction = Normalize(mDirection + (u * posXNorm) - (v * posYNorm));
    return result;
}



void Camera::MakeProjection() {
    const float aspect = this->GetAspect();
    mProjection = MatPerspective(Deg2Rad(mFovY), aspect, mNearZ, mFarZ);
}

void Camera::MakeTransform() {
    mTransform = MatLookAt(mPosition, mPosition + mDirection, sCameraUp);
}

} // namespace u4a
