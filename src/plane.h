//
// Created by Nahmi on 2026-06-04.
//

#ifndef HIGHPERF_RAYTRACER_PLANE_H
#define HIGHPERF_RAYTRACER_PLANE_H

#include "hittable.h"

class Plane : public Hittable {
public:
    Point3 point;   // a point on the plane
    Vec3   normal;  // the plane's normal (should be normalized)
    Material *mat;

    Plane(Point3 point, Vec3 normal, Material *mat)
        : point(point), normal(normal.normalized()), mat(mat) {}

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        double denom = normal.dot(r.dir);

        // ray is parallel to the plane — no hit
        if (std::fabs(denom) < 1e-8) return false;

        double t = (point - r.origin).dot(normal) / denom;
        if (t < tMin || t > tMax) return false;

        rec.t        = t;
        rec.point    = r.at(t);
        rec.normal   = normal;
        rec.material = mat;
        return true;
    }

    // infinite plane — return a huge bounding box
    AABB boundingBox() const override {
        const double big = 1e9;
        return AABB{
            {-big, point.y - 0.0001, -big},
            { big, point.y + 0.0001,  big}
        };
    }
};

#endif //HIGHPERF_RAYTRACER_PLANE_H
