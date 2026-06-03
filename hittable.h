//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_HITTABLE_H
#define HIGHPERF_RAYTRACER_HITTABLE_H

#include "main.h"

struct HitRecord {
    Point3 point;
    Vec3 normal;
    double t;
};

class Hittable {
public:
    virtual bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const = 0;

    virtual ~Hittable() = default;
};

#endif //HIGHPERF_RAYTRACER_HITTABLE_H
