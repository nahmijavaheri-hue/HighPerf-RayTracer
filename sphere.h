//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_SPHERE_H
#define HIGHPERF_RAYTRACER_SPHERE_H

#include "hittable.h"

class Sphere : public Hittable {
public:
    Point3 center;
    double radius;

    Sphere(Point3 center, double radius) : center(center), radius(radius) {
    }

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        Vec3 oc = r.origin - center;
        double a = r.dir.dot(r.dir);
        double b = 2.0 * oc.dot(r.dir);
        double c = oc.dot(oc) - radius * radius;
        double disc = b * b - 4 * a * c;
        if (disc < 0) return false;

        double t = (-b - std::sqrt(disc)) / (2.0 * a);
        if (t < tMin || t > tMax) {
            t = (-b + std::sqrt(disc)) / (2.0 * a);
            if (t < tMin || t > tMax) return false;
        }

        rec.t = t;
        rec.point = r.at(t);
        rec.normal = (rec.point - center) / radius;
        return true;
    }
};

#endif //HIGHPERF_RAYTRACER_SPHERE_H
