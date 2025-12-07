#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <math.h>
#include "scene.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline Vec3 v_sub(Vec3 a, Vec3 b) {
    Vec3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
    return r;
}

static inline Vec3 v_add(Vec3 a, Vec3 b) {
    Vec3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

static inline Vec3 v_scale(Vec3 a, float s) {
    Vec3 r = {a.x * s, a.y * s, a.z * s};
    return r;
}

static inline float v_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 v_cross(Vec3 a, Vec3 b) {
    Vec3 r = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
    return r;
}

static inline Vec3 v_normalize(Vec3 a) {
    float len = sqrt(v_dot(a, a));
    if (len == 0) return a;
    return v_scale(a, 1.0f / len);
}

static inline Vec3 v_neg(Vec3 a) {
    Vec3 r = {-a.x, -a.y, -a.z};
    return r;
}

static inline Vec3 v_mul(Vec3 a, Vec3 b) {
    Vec3 r = {a.x * b.x, a.y * b.y, a.z * b.z};
    return r;
}

static inline float v_length(Vec3 a) {
    return sqrt(v_dot(a, a));
}

static inline Vec3 v_clamp(Vec3 c) {
    Vec3 r;
    r.x = c.x > 1.0f ? 1.0f : (c.x < 0.0f ? 0.0f : c.x);
    r.y = c.y > 1.0f ? 1.0f : (c.y < 0.0f ? 0.0f : c.y);
    r.z = c.z > 1.0f ? 1.0f : (c.z < 0.0f ? 0.0f : c.z);
    return r;
}

#endif