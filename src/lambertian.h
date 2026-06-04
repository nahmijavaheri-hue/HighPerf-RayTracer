//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_LAMBERTIAN_H
#define HIGHPERF_RAYTRACER_LAMBERTIAN_H

#include "hittable.h"
#include "material.h"

class Lambertian : public Material {
private:
    Color albedo;

public:
    explicit Lambertian(const Color &albedo) {
        this->albedo = albedo;
    };

    bool scatter(const Ray &in, const HitRecord &rec, Color &attenuation, Ray &scattered) const override {
        scattered.origin = rec.point;
        scattered.dir = (rec.normal + Vec3{
                             generateRandomOffset(), generateRandomOffset(), generateRandomOffset()
                         }).normalized();
        attenuation = albedo;
        return true;
    }
};

#endif //HIGHPERF_RAYTRACER_LAMBERTIAN_H
