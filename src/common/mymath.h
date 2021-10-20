#pragma once
#include "mycommon.h"

#pragma warning(push)
#pragma warning(disable : 4201) // C4201: nonstandard extension used: nameless struct/union
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SIZE_T_LENGTH
//#NOTE_SK: I'm using Dx11 now instead of OpenGL, so can use [0, 1] depth
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

using vec2 = glm::highp_vec2;
using vec3 = glm::highp_vec3;
using vec4 = glm::highp_vec4;
using mat3 = glm::highp_mat3;
using mat4 = glm::highp_mat4;
using mat3x4 = glm::highp_mat3x4;
using mat4x3 = glm::highp_mat4x3;
using quat = glm::highp_quat;

using ivec4 = glm::highp_ivec4;
using uvec4 = glm::highp_uvec4;

using vec4s16 = glm::highp_i16vec4;

struct anglef : public glm::highp_vec1 {};
struct ang3f : public vec3 {};

struct color4f : public vec4 {
    color4f() : vec4() {}
    color4f(const color4f& other) : vec4(*rcast<const vec4*>(&other)) {}
    color4f(const float _r, const float _g, const float _b, const float _a) : vec4(_r, _g, _b, _a) {}
};
struct color32u { uint32_t value; };

struct pose_43 : mat4x3 {};
struct pose_43T : mat3x4 {};

static const float MM_Pi = 3.14159265358979323846f;
static const float MM_InvPi = 1.0f / MM_Pi;
static const float MM_TwoPi = MM_Pi * 2.0f;
static const float MM_HalfPi = MM_Pi * 0.5f;
static const float MM_Epsilon = 1.192092896e-07f;
static const float MM_OneMinusEpsilon = 0.9999999403953552f;


inline float Rad2Deg(const float rad) {
    return rad * (180.0f / MM_Pi);
}

inline float Deg2Rad(const float deg) {
    return deg * (MM_Pi / 180.0f);
}

template <typename T>
inline T Min3(const T& a, const T& b, const T& c) {
    return std::min(a, std::min(b, c));
}

template <typename T>
inline T Max3(const T& a, const T& b, const T& c) {
    return std::max(a, std::max(b, c));
}

inline float Min3(const vec3& v) {
    return Min3(v.x, v.y, v.z);
}

inline float Max3(const vec3& v) {
    return Max3(v.x, v.y, v.z);
}

template <typename T>
inline T Lerp(const T& a, const T& b, const float t) {
    //return a + (b - a) * t;
    return (a * (1.0f - t)) + (b * t);
}

template <typename T>
inline T Clamp(const T& x, const T& left, const T& right) {
    T result = x;
    if (x < left) {
        result = left;
    } else if (x > right) {
        result = right;
    }
    return result;
}

struct fp32_q8 {
    float value;

    inline void SetAsU8(const uint8_t u8) {
        this->value = scast<float>(u8) * (1.0f / 255.0f) * 2.0f;
    }

    inline uint8_t GetAsU8() const {
        return scast<uint8_t>(Clamp(this->value * 0.5f * 255.0f, 0.0f, 255.0f));
    }
};

inline float Sin(const float x) {
    return std::sinf(x);
}

inline float Cos(const float x) {
    return std::cosf(x);
}

inline float Sqrt(const float x) {
    return std::sqrtf(x);
}

inline int32_t Floori(const float x) {
    return static_cast<int32_t>(floorf(x));
}


template <typename T>
inline float Length(const T& v) {
    return glm::length(v);
}

template <typename T>
inline float Distance(const T& a, const T& b) {
    return glm::length(a - b);
}

template <typename T>
inline float LengthSqr(const T& v) {
    return glm::dot(v, v);
}

inline float Dot(const vec2& a, const vec2& b) {
    return glm::dot(a, b);
}
inline float Dot(const vec3& a, const vec3& b) {
    return glm::dot(a, b);
}
inline float Dot(const vec4& a, const vec4& b) {
    return glm::dot(a, b);
}

inline vec3 Cross(const vec3& a, const vec3& b) {
    return glm::cross(a, b);
}

inline vec3 Normalize(const vec3& v) {
    return glm::normalize(v);
}
inline quat Normalize(const quat& q) {
    return glm::normalize(q);
}

inline float MaxComponent(const vec2& v) {
    return std::max(v.x, v.y);
}
inline float MaxComponent(const vec3& v) {
    return std::max(v.x, std::max(v.y, v.z));
}
inline float MaxComponent(const vec4& v) {
    return std::max(v.x, std::max(v.y, std::max(v.z, v.w)));
}
inline float MinComponent(const vec2& v) {
    return std::min(v.x, v.y);
}
inline float MinComponent(const vec3& v) {
    return std::min(v.x, std::min(v.y, v.z));
}
inline float MinComponent(const vec4& v) {
    return std::min(v.x, std::min(v.y, std::min(v.z, v.w)));
}

inline quat QuatAngleAxis(const float angleRad, const vec3& axis) {
    return glm::angleAxis(angleRad, axis);
}
inline vec3 QuatRotate(const quat& q, const vec3& v) {
    return glm::rotate(q, v);
}
inline quat QuatSlerp(const quat& a, const quat& b, const float t) {
    return glm::slerp(a, b, t);
}
inline vec3 QuatToEuler(const quat& q) {
    return glm::eulerAngles(q);
}
inline quat QuatFromEuler(const vec3& euler) {
    return quat(euler);
}
inline quat QuatFromDirection(const vec3& dir) {
    const float angle = atan2f(dir.z, dir.x);
    const float qx = 0.0f;
    const float qy = sinf(angle * 0.5f);
    const float qz = 0.0f;
    const float qw = cosf(angle * 0.5f);
    return quat(qw, qx, qy, qz);
}
inline quat QuatConjugate(const quat& q) {
    return glm::conjugate(q);
}

inline mat4 MatRotate(const float angle, const float x, const float y, const float z) {
    return glm::rotate(angle, vec3(x, y, z));
}
inline mat4 MatTranslate(const mat4& m, const vec3& v) {
    return glm::translate(m, v);
}
inline mat4 MatOrtho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
    return glm::orthoLH(left, right, bottom, top, zNear, zFar);
}
inline mat4 MatPerspective(const float fovy, const float aspect, const float zNear, const float zFar) {
    return glm::perspectiveLH(fovy, aspect, zNear, zFar);
}
inline mat4 MatLookAt(const vec3& eye, const vec3& center, const vec3& up) {
    return glm::lookAtLH(eye, center, up);
}
inline mat4 MatInverse(const mat4& m) {
    return glm::inverse(m);
}
inline mat4 MatTranspose(const mat4& m) {
    return glm::transpose(m);
}
inline mat4 MatFromQuat(const quat& q) {
    return glm::toMat4(q);
}
inline mat4 MatFromEuler(const vec3& euler) {
    return glm::eulerAngleZYX(euler.z, euler.y, euler.x);
}
inline mat4 MatFromPose(const pose_43& m43) {
#if 0
    mat4 m;
    m[0] = vec4(m43[0], 0.0f);
    m[1] = vec4(m43[1], 0.0f);
    m[2] = vec4(m43[2], 0.0f);
    m[3] = vec4(m43[3], 1.0f);
    return m;
#else
    return mat4(m43);
#endif
}
inline mat4 MatFromPose(const pose_43T& m43T) {
    mat4 m;
#if 1
    m[0] = vec4(m43T[0].x, m43T[1].x, m43T[2].x, 0.0f);
    m[1] = vec4(m43T[0].y, m43T[1].y, m43T[2].y, 0.0f);
    m[2] = vec4(m43T[0].z, m43T[1].z, m43T[2].z, 0.0f);
    m[3] = vec4(m43T[0].w, m43T[1].w, m43T[2].w, 1.0f);
#else
    m[0] = vec4(vec3(m43T[0]), 0.0f);
    m[1] = vec4(vec3(m43T[1]), 0.0f);
    m[2] = vec4(vec3(m43T[2]), 0.0f);
    m[3] = vec4(m43T[0].w, m43T[1].w, m43T[2].w, 1.0f);
#endif
    return m;
}
inline void MatDecompose(const mat4& m, vec3& offset, vec3& scale, quat& rotation) {
    quat _rotation;
    vec3 skew;
    vec4 persp;
    glm::decompose(m, scale, _rotation, offset, skew, persp);
    //Keep in mind that the resulting quaternion in not correct. It returns its conjugate!
    rotation = glm::conjugate(_rotation);
}
inline const float* MatToPtr(const mat4& m) {
    return rcast<const float*>(&m);
}
inline float* MatToPtrMutable(mat4& m) {
    return rcast<float*>(&m);
}

static const mat4 MatZero = mat4(0.0f);
static const mat4 MatIdentity = mat4(1.0f);

using Mat4Array = MyArray<mat4>;

// http://jcgt.org/published/0006/01/01/
// branchlessONB
inline void OrthonormalBasis(const vec3& n, vec3& b1, vec3& b2) {
    const float sign = std::copysignf(1.0f, n.z);
    const float a = -1.0f / (sign + n.z);
    const float b = n.x * n.y * a;
    b1 = vec3(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
    b2 = vec3(b, sign + n.y * n.y * a, -n.y);
}



inline float DistanceFromPlane(const vec4& plane, const vec3& point) {
    return Dot(plane, vec4(point, 1.0f));
}


constexpr color32u ColorRGBA_32U(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
    return {
        (scast<uint32_t>(a) << 24) |
        (scast<uint32_t>(b) << 16) |
        (scast<uint32_t>(g) <<  8) |
        (scast<uint32_t>(r) <<  0)
    };
}

constexpr color32u Color4FTo32U(const color4f& color) {
    return ColorRGBA_32U(scast<uint8_t>(Clamp(color.r * 255.0f, 0.0f, 255.0f)),
                         scast<uint8_t>(Clamp(color.g * 255.0f, 0.0f, 255.0f)),
                         scast<uint8_t>(Clamp(color.b * 255.0f, 0.0f, 255.0f)),
                         scast<uint8_t>(Clamp(color.a * 255.0f, 0.0f, 255.0f)));
}


struct AABBox {
    vec3 minimum, maximum;

    inline bool Valid() const {
        return Length(this->Extent()) > MM_Epsilon;
    }

    inline void Reset(const bool makeEmpty = false) {
        minimum = makeEmpty ? vec3(0.0f) : vec3(10000.0f);
        maximum = makeEmpty ? vec3(0.0f) : vec3(-10000.0f);
    }

    inline void Absorb(const vec3& p) {
        minimum.x = std::min(minimum.x, p.x);
        minimum.y = std::min(minimum.y, p.y);
        minimum.z = std::min(minimum.z, p.z);

        maximum.x = std::max(maximum.x, p.x);
        maximum.y = std::max(maximum.y, p.y);
        maximum.z = std::max(maximum.z, p.z);
    }

    inline void Absorb(const AABBox& other) {
        this->Absorb(other.minimum);
        this->Absorb(other.maximum);
    }

    inline vec3 Center() const {
        return (minimum + maximum) * 0.5f;
    }

    inline vec3 Extent() const {
        return (maximum - minimum) * 0.5f;
    }

    inline float MaximumValue() const {
        return std::max(Max3(std::fabsf(minimum.x), std::fabsf(minimum.y), std::fabsf(minimum.z)),
                        Max3(std::fabsf(maximum.x), std::fabsf(maximum.y), std::fabsf(maximum.z)));
    }
};

struct BSphere {
    vec3    center;
    float   radius;

    inline bool Valid() const {
        return this->radius > MM_Epsilon;
    }

    inline void Reset() {
        this->center = vec3(0.0f);
        this->radius = 0.0f;
    }

    inline void Absorb(const vec3& p) {
        vec3 posDelta = p - this->center;
        const float testRadius = Length(posDelta);
        if (testRadius > this->radius) {
            const float radiusDiff = (testRadius - this->radius) * 0.5f;
            this->center += posDelta * (radiusDiff / testRadius);
            this->radius += radiusDiff;
        }
    }

    inline void Absorb(const BSphere& other) {
        //#NOTE_SK: 1-st step - we check if the other sphere is inside this one
        // Calculate d == The distance between the sphere centers
        const float distanceCenters = Distance(this->center, other.center);

        if (distanceCenters + other.radius <= this->radius) {
            return;
        }

        //#NOTE_SK: the other way around - the other sphere contains this one
        if (distanceCenters + this->radius <= other.radius) {
            this->center = other.center;
            this->radius = other.radius;
            return;
        }

        //#NOTE_SK: now simply make new sphere that contains both
        const float newRadius = (this->radius + distanceCenters + other.radius) * 0.5f;
        const float ratio = (newRadius - this->radius) / distanceCenters;

        this->center += (other.center - this->center) * ratio;
        this->radius = newRadius;
    }
};

struct Frustum {
    enum Plane : size_t {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        NumPlanes
    };

    vec4 planes[Plane::NumPlanes];

    void FromMatrix(const mat4& m) {
        const mat4& mt = MatTranspose(m);
        this->planes[Plane::Left]   = mt[3] + mt[0];
        this->planes[Plane::Right]  = mt[3] - mt[0];
        this->planes[Plane::Bottom] = mt[3] + mt[1];
        this->planes[Plane::Top]    = mt[3] - mt[1];
        this->planes[Plane::Near]   = mt[3] + mt[2];
        this->planes[Plane::Far]    = mt[3] - mt[2];
    }

    bool IsBSphereIn(const BSphere& sphere) const {
        // Loop through each side of the frustum and test if the sphere lies outside any of them.
        for (size_t i = 0; i < Plane::NumPlanes; ++i) {
            const float d = DistanceFromPlane(this->planes[i], sphere.center);
            if (d < -sphere.radius) {
                return false;
            }
        }
        return true;
    }

    // http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
    bool IsAABBoxIn(const AABBox& bbox) const {
        // check box outside/inside of frustum
        for (size_t i = 0; i < Plane::NumPlanes; ++i) {
            if ((DistanceFromPlane(this->planes[i], vec3(bbox.minimum.x, bbox.minimum.y, bbox.minimum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.maximum.x, bbox.minimum.y, bbox.minimum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.minimum.x, bbox.maximum.y, bbox.minimum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.maximum.x, bbox.maximum.y, bbox.minimum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.minimum.x, bbox.minimum.y, bbox.maximum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.maximum.x, bbox.minimum.y, bbox.maximum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.minimum.x, bbox.maximum.y, bbox.maximum.z)) < 0.0) &&
                (DistanceFromPlane(this->planes[i], vec3(bbox.maximum.x, bbox.maximum.y, bbox.maximum.z)) < 0.0)) {
                return false;
            }
        }
        return true;
    }
};

struct Ray {
    vec3    origin;
    vec3    direction;
};

inline Ray GetRayFromScreenPos(const vec2& pt, const vec2& screenSize, const mat4& proj, const mat4& mv) {
    // Compute the vector of the pick ray in screen space
    vec3 v(-(((2.0f * pt.x) / screenSize.x) - 1.0f) / proj[0][0],
            (((2.0f * pt.y) / screenSize.y) - 1.0f) / proj[1][1],
            1.0f);

    // Get the inverse view matrix
    mat4 m = MatInverse(mv);

    // Transform the screen space pick ray into 3D space
    Ray result;
    result.direction.x = -(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2]);
    result.direction.y = -(v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2]);
    result.direction.z = -(v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2]);
    result.origin.x = m[0][3];
    result.origin.y = m[1][3];
    result.origin.z = m[2][3];

    return result;
}

inline bool RayAABBIntersection(const Ray& ray, const AABBox& aabb) {
    float tmin = (aabb.minimum.x - ray.origin.x) / ray.direction.x;
    float tmax = (aabb.maximum.x - ray.origin.x) / ray.direction.x;

    if (tmin > tmax) {
        std::swap(tmin, tmax);
    }

    float tymin = (aabb.minimum.y - ray.origin.y) / ray.direction.y;
    float tymax = (aabb.maximum.y - ray.origin.y) / ray.direction.y;

    if (tymin > tymax) {
        std::swap(tymin, tymax);
    }

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin) {
        tmin = tymin;
    }

    if (tymax < tmax) {
        tmax = tymax;
    }

    float tzmin = (aabb.minimum.z - ray.origin.z) / ray.direction.z;
    float tzmax = (aabb.maximum.z - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax) {
        std::swap(tzmin, tzmax);
    }

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    if (tzmin > tmin) {
        tmin = tzmin;
    }

    if (tzmax < tmax) {
        tmax = tzmax;
    }

    return true;
}
