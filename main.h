#ifndef HIGHPERF_RAYTRACER_MAIN_H
#define HIGHPERF_RAYTRACER_MAIN_H

#include <cmath>
#include <iostream>

// ── Vec3 ────────────────────────────────────────────────────────────────────

struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {
    }

    Vec3(double x, double y, double z) : x(x), y(y), z(z) {
    }

    Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(double t) const { return {x * t, y * t, z * t}; }
    Vec3 operator*(const Vec3 &v) const { return {x * v.x, y * v.y, z * v.z}; }
    Vec3 operator/(double t) const { return *this * (1.0 / t); }
    Vec3 operator-() const { return {-x, -y, -z}; }

    double dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
    double lengthSq() const { return dot(*this); }
    double length() const { return std::sqrt(lengthSq()); }
    Vec3 normalized() const { return *this / length(); }
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
    out << static_cast<int>(255.99 * c.x) << ' '
            << static_cast<int>(255.99 * c.y) << ' '
            << static_cast<int>(255.99 * c.z) << '\n';
}

#endif // HIGHPERF_RAYTRACER_MAIN_H
