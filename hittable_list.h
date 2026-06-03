//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_HITTABLE_LIST_H
#define HIGHPERF_RAYTRACER_HITTABLE_LIST_H

#include <vector>
#include "hittable.h"

class HittableList : public Hittable {
public:
    std::vector<Hittable *> objects;

    void add(Hittable *obj) { objects.push_back(obj); }

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        HitRecord temp;
        bool hitAnything = false;
        double closest = tMax;

        for (auto obj: objects) {
            if (obj->hit(r, tMin, closest, temp)) {
                hitAnything = true;
                closest = temp.t;
                rec = temp;
            }
        }
        return hitAnything;
    }
};

#endif //HIGHPERF_RAYTRACER_HITTABLE_LIST_H
