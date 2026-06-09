//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_HITTABLE_LIST_H
#define HIGHPERF_RAYTRACER_HITTABLE_LIST_H

#include <vector>
#include "hittable.h"

inline AABB surroundingBox(const AABB& a, const AABB& b) {
    return AABB{
        {std::fmin(a.min.x, b.min.x), std::fmin(a.min.y, b.min.y), std::fmin(a.min.z, b.min.z)},
        {std::fmax(a.max.x, b.max.x), std::fmax(a.max.y, b.max.y), std::fmax(a.max.z, b.max.z)}
    };
}

class HittableList : public Hittable {
public:
    std::vector<Hittable *> objects;

    void add(Hittable *obj) { objects.push_back(obj); }

    AABB boundingBox() const override {
        AABB box = objects[0]->boundingBox();
        for (int i = 1; i < objects.size(); i++)
            box = surroundingBox(box, objects[i]->boundingBox());
        return box;
    }

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
