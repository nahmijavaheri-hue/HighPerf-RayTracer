//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_AABB_H
#define HIGHPERF_RAYTRACER_AABB_H
#include "hittable.h"
#include "main.h"

struct AABB {
    Point3 min;
    Point3 max;

    bool hit(const Ray &r, double tMin, double tMax) const {
        for (int i = 0; i < 3; i++) {
            //t = (position - origin) / direction
            double inverseD = 1.0 / (i == 0 ? r.dir.x : i == 1 ? r.dir.y : r.dir.z);
            double origin = (i == 0 ? r.origin.x : i == 1 ? r.origin.y : r.origin.z);
            double minVal = (i == 0 ? min.x : i == 1 ? min.y : min.z);
            double maxVal = (i == 0 ? max.x : i == 1 ? max.y : max.z);
            double t_min = (minVal - origin) * inverseD;
            double t_max = (maxVal - origin) * inverseD;
            if (inverseD < 0.0) std::swap(t_min, t_max);

            tMin = t_min > tMin ? t_min : tMin;
            tMax = t_max < tMax ? t_max : tMax;
            if (tMax <= tMin) return false;
        }
        return true;
    }
};

#endif //HIGHPERF_RAYTRACER_AABB_H