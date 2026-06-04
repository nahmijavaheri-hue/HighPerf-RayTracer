//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_HITTABLE_H
#define HIGHPERF_RAYTRACER_HITTABLE_H

#include "main.h"
#include "material.h"

struct HitRecord {
    Point3 point; // Point at which hit occurred
    Vec3 normal; // Direction
    double t{}; // Distance at which hit occurred
    Material* material{}; // Material pointer
};

class Hittable {
public:
    virtual bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const = 0;

    virtual ~Hittable() = default;
};

#endif //HIGHPERF_RAYTRACER_HITTABLE_H
