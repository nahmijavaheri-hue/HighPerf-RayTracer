//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_DIELECTRIC_H
#define HIGHPERF_RAYTRACER_DIELECTRIC_H

#include "material.h"
#include "hittable.h"

class Dielectric : public Material {
private:
    double refractionIndex;
public:
    Dielectric(double refractionIndex) : refractionIndex(refractionIndex) {}

    bool scatter(const Ray &in, const HitRecord &rec, Color &attenuation, Ray &scattered) const override {
        attenuation = {1, 1, 1}; // glass absorbs nothing

        // are we inside or outside the surface?
        bool frontFace = in.dir.dot(rec.normal) < 0;
        Vec3 normal = frontFace ? rec.normal : -rec.normal;
        double ratio = frontFace ? (1.0 / refractionIndex) : refractionIndex;

        Vec3 unitDir = in.dir.normalized();
        double cosTheta = (-unitDir).dot(normal);
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        // total internal reflection check
        bool cannotRefract = ratio * sinTheta > 1.0;

        Vec3 outDir;
        if (cannotRefract || schlick(cosTheta, ratio) > generateRandomOffset()) {
            // reflect
            outDir = unitDir - normal * 2 * unitDir.dot(normal);
        } else {
            // refract (Snell's law in vector form)
            Vec3 perpendicular = (unitDir + normal * cosTheta) * ratio;
            Vec3 parallel = normal * (-std::sqrt(1.0 - perpendicular.lengthSq()));
            outDir = perpendicular + parallel;
        }

        scattered.origin = rec.point;
        scattered.dir = outDir;
        return true;
    }

private:
    static double schlick(double cosine, double ratio) {
        double r0 = (1 - ratio) / (1 + ratio);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow(1 - cosine, 5);
    }
};

#endif //HIGHPERF_RAYTRACER_DIELECTRIC_H