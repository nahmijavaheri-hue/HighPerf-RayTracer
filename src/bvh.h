//
// Created by Nahmi on 2026-06-03.
//

#ifndef HIGHPERF_RAYTRACER_BVH_H
#define HIGHPERF_RAYTRACER_BVH_H

#include <vector>
#include <algorithm>
#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"

class BVHNode : public Hittable {
public:
    Hittable *left;
    Hittable *right;
    AABB box;

    BVHNode(std::vector<Hittable *> &objects, int start, int end) {
        int axis = static_cast<int>(3 * generateRandomOffset());

        auto center = [axis](const Hittable *h) {
            AABB b = h->boundingBox();
            return axis == 0 ? b.min.x + b.max.x : axis == 1 ? b.min.y + b.max.y : b.min.z + b.max.z;
        };
        auto comparator = [&center](const Hittable *a, const Hittable *b) { return center(a) < center(b); };

        int span = end - start;

        if (span == 1) {
            left = right = objects[start];
        } else if (span == 2) {
            if (comparator(objects[start], objects[start + 1])) {
                left = objects[start];
                right = objects[start + 1];
            } else {
                left = objects[start + 1];
                right = objects[start];
            }
        } else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            int mid = start + span / 2;
            left = new BVHNode(objects, start, mid);
            right = new BVHNode(objects, mid, end);
        }

        box = surroundingBox(left->boundingBox(), right->boundingBox());
    }

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        if (!box.hit(r, tMin, tMax)) return false;

        bool hitLeft = left->hit(r, tMin, tMax, rec);
        bool hitRight = right->hit(r, tMin, hitLeft ? rec.t : tMax, rec);

        return hitLeft || hitRight;
    }

    AABB boundingBox() const override {
        return box; // return the stored box
    }
};

#endif //HIGHPERF_RAYTRACER_BVH_H