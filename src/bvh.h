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
        // 1. pick a random axis to sort along
        // 2. sort objects in [start, end) by their bounding box center on that axis
        // 3. if one object:  assign to both left and right
        //    if two objects: assign one to left, one to right
        //    else:           split in half, recurse into left and right
        // 4. compute this node's bounding box from left and right
    }

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        // 1. test ray against this node's bounding box — if miss, return false
        // 2. recurse into left and right, keeping the closest hit
    }

    AABB boundingBox() const override {
        return box; // return the stored box
    }
};

#endif //HIGHPERF_RAYTRACER_BVH_H