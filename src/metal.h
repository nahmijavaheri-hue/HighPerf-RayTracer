//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_METAL_H
#define HIGHPERF_RAYTRACER_METAL_H

#include "hittable.h"
#include "material.h"

class Metal : public Material {
private:
    Color albedo;
    double fuzz; // 0.0 <= fuzz <= 1.0
public:
    Metal(const Color &albedo, double fuzz) {
        this->albedo = albedo;
        this->fuzz = fuzz;
    };

    bool scatter(const Ray &in, const HitRecord &rec, Color &attenuation, Ray &scattered) const override {
        Vec3 reflected = in.dir.normalized() - rec.normal * 2 * in.dir.normalized().dot(rec.normal);
        scattered.origin = rec.point;
        scattered.dir = reflected + Vec3{generateRandomOffset(), generateRandomOffset(), generateRandomOffset()} * fuzz;
        attenuation = albedo;
        return scattered.dir.dot(rec.normal) > 0;
    }
};
#endif //HIGHPERF_RAYTRACER_METAL_H
