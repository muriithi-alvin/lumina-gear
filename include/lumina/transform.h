// transform.h - 2D affine transformation matrices for CP2.
// Provides 3x3 matrices for translation, rotation, scaling, and compound transforms.
#pragma once

#include <cmath>

namespace lumina {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

inline Vec2 operator+(const Vec2& a, const Vec2& b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(const Vec2& a, float s) { return {a.x * s, a.y * s}; }

// 3x3 matrix for 2D affine transformations (homogeneous coordinates):
// [ m[0][0]  m[0][1]  m[0][2] ]   [ x ]
// [ m[1][0]  m[1][1]  m[1][2] ] * [ y ]
// [   0        0        1     ]   [ 1 ]
struct Mat3 {
    float m[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };
};

inline Mat3 mat3Identity() {
    return Mat3();
}

inline Mat3 mat3Translate(float tx, float ty) {
    Mat3 res;
    res.m[0][2] = tx;
    res.m[1][2] = ty;
    return res;
}

inline Mat3 mat3Rotate(float radians) {
    Mat3 res;
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    res.m[0][0] = c;
    res.m[0][1] = -s;
    res.m[1][0] = s;
    res.m[1][1] = c;
    return res;
}

inline Mat3 mat3Scale(float sx, float sy) {
    Mat3 res;
    res.m[0][0] = sx;
    res.m[1][1] = sy;
    return res;
}

inline Mat3 mat3Multiply(const Mat3& a, const Mat3& b) {
    Mat3 res;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            res.m[r][c] = a.m[r][0] * b.m[0][c] +
                          a.m[r][1] * b.m[1][c] +
                          a.m[r][2] * b.m[2][c];
        }
    }
    return res;
}

inline Vec2 mat3TransformPoint(const Mat3& m, const Vec2& p) {
    return {
        m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2],
        m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2]
    };
}

// Compound transform: rotate around an arbitrary pivot point (cx, cy)
// M = T(cx, cy) * R(radians) * T(-cx, -cy)
inline Mat3 mat3RotateAround(float radians, float cx, float cy) {
    const Mat3 tToOrigin = mat3Translate(-cx, -cy);
    const Mat3 rot = mat3Rotate(radians);
    const Mat3 tBack = mat3Translate(cx, cy);
    return mat3Multiply(tBack, mat3Multiply(rot, tToOrigin));
}

}  // namespace lumina

