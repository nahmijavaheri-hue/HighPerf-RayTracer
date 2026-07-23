#ifndef HIGHPERF_RAYTRACER_MAIN_H
#define HIGHPERF_RAYTRACER_MAIN_H

#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>

// ── Vec3 ────────────────────────────────────────────────────────────────────

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {
    }

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {
    }

    Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(const float t) const { return {x * t, y * t, z * t}; }
    Vec3 operator*(const Vec3 &v) const { return {x * v.x, y * v.y, z * v.z}; }
    Vec3 operator/(const float t) const { return *this * (1 / t); }
    Vec3 operator-() const { return {-x, -y, -z}; }

    float dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
    float lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSq()); }
    Vec3 normalized() const { return *this / length(); }

    Vec3 &operator+=(const Vec3 &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
};

using Point3 = Vec3;
using Color = Vec3;

// ── Ray ─────────────────────────────────────────────────────────────────────

struct Ray {
    Point3 origin;
    Vec3 dir;

    Point3 at(double t) const { return origin + dir * t; }
};

// ── PPM output ───────────────────────────────────────────────────────────────

inline void writeColor(std::ostream &out, Color c) {
    out << static_cast<int>(255.99 * sqrt(std::clamp(c.x, 0.0f, 1.0f))) << ' '
            << static_cast<int>(255.99 * sqrt(std::clamp(c.y, 0.0f, 1.0f))) << ' '
            << static_cast<int>(255.99 * sqrt(std::clamp(c.z, 0.0f, 1.0f))) << '\n';
}

inline double generateRandomOffset() {
    thread_local std::mt19937_64 engine(std::random_device{}());
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(engine);
}

#endif // HIGHPERF_RAYTRACER_MAIN_H
