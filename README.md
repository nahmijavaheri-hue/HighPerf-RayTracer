# HighPerf RayTracer

A physically-based ray tracer written from scratch in C++, with no external rendering libraries. Built for correctness first, performance second — every system is hand-rolled, from the math primitives up to BVH acceleration.

---

## Current Render

> 1600×900 — 150 samples per pixel — 50 max bounces

![Current Render](render.png)

Three spheres on an infinite plane: a diffuse Lambertian on the right, a perfect mirror metal in the center, and a glass dielectric on the left — rendered with a blue-white sky gradient background.

---

## Architecture

The renderer is structured around a clean virtual `Hittable` interface. Every object in the scene exposes two methods — `hit()` for ray intersection and `boundingBox()` for BVH traversal — making it trivial to add new primitives.

```
src/
├── main.h            — Vec3, Ray, Color, PPM output, RNG
├── main.cpp          — camera, render loop, scene setup
├── hittable.h        — Hittable base class + HitRecord
├── hittable_list.h   — scene container + surroundingBox()
├── aabb.h            — axis-aligned bounding box (slab method)
├── bvh.h             — BVH node (in progress)
├── sphere.h          — sphere primitive
├── plane.h           — infinite plane primitive
├── material.h        — Material base class
├── lambertian.h      — diffuse Lambertian scattering
├── metal.h           — specular reflection with fuzz
└── dielectric.h      — glass refraction (Snell's law + Schlick)
```

---

## What's Implemented

### Core
- **Vec3 / Ray / Color** — hand-rolled math with dot product, normalization, arithmetic operators
- **PPM output** — renders directly to `.ppm` image files
- **Recursive ray tracing** — up to N bounces with configurable depth
- **Anti-aliasing** — multi-sample per pixel with random jitter

### Primitives
- **Sphere** — analytic ray-sphere intersection
- **Infinite Plane** — ray-plane intersection with a configurable normal

### Materials
- **Lambertian** — diffuse scattering with random hemisphere sampling
- **Metal** — specular reflection with a fuzz parameter for roughness
- **Dielectric** — full refraction via Snell's law in vector form, total internal reflection, and Schlick's approximation for Fresnel reflectance

### Acceleration
- **AABB** — axis-aligned bounding box with slab-method ray intersection
- **BVH scaffolding** — `BVHNode` class inheriting from `Hittable`, structure in place for recursive tree traversal

---

## In Progress

### BVH Construction & Traversal
The `BVHNode` constructor and `hit()` method are stubbed out and ready to implement. The strategy:
- **Longest-axis split** — at each node, sort objects along whichever axis (X/Y/Z) has the widest spatial spread, then split at the midpoint
- **Recursive build** — left and right subtrees built recursively until leaf nodes hold single objects
- Expected result: ray intersection drops from **O(n)** to **O(log n)**

---

## Roadmap

### Short Term
- [ ] Complete BVH constructor and `hit()` traversal
- [ ] Gamma correction (currently linear output)
- [ ] Moveable / configurable camera (FOV, look-at, aperture)
- [ ] Depth of field / defocus blur

### Materials & Lighting
- [ ] Emissive materials (area lights)
- [ ] Texture mapping (UV coordinates, image textures, procedural noise)
- [ ] Physically correct BRDF (microfacet model)

### Primitives
- [ ] Triangle mesh support (OBJ loading)
- [ ] Axis-aligned boxes
- [ ] Motion blur (time-sampled rays)

### Performance
- [ ] Multithreading (std::thread tile-based rendering)
- [ ] SIMD vectorization for AABB and Vec3 operations
- [ ] BVH SAH (Surface Area Heuristic) for optimal tree splits
- [ ] Progressive rendering with live preview

### Long Term
- [ ] GPU port (CUDA or Vulkan compute)
- [ ] Spectral rendering (wavelength-based instead of RGB)
- [ ] Volumetric rendering (participating media, fog, subsurface scattering)
- [ ] Full path tracer with multiple importance sampling (MIS)

---

## Building

Requires a C++17 compiler and CMake.

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
./cmake-build-debug/HighPerf-RayTracer
```

Output is written to `src/image.ppm`. Open with any PPM viewer or convert with ImageMagick:

```bash
magick src/image.ppm output.png
```

---

## References

- [_Ray Tracing in One Weekend_](https://raytracing.github.io/books/RayTracingInOneWeekend.html) — Peter Shirley
- [_Physically Based Rendering_](https://pbr-book.org) — Pharr, Jakob, Humphreys
- Schlick, C. (1994). *An Inexpensive BRDF Model for Physically-based Rendering*
