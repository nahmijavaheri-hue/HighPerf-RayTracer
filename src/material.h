//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_MATERIAL_H
#define HIGHPERF_RAYTRACER_MATERIAL_H

#include "main.h"
struct Ray;
struct HitRecord;

class Material {
private:
public:
    virtual ~Material() = default;

    virtual bool scatter(const Ray& in, const HitRecord& rec, Color& attenuation, Ray& scattered) const = 0;
};
#endif //HIGHPERF_RAYTRACER_MATERIAL_H