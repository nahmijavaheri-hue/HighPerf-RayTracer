#include "main.h"
#include "hittable_list.h"
#include "sphere.h"
#include <fstream>
#include <limits>

const double infinity = std::numeric_limits<double>::infinity();

Color rayColor(const Ray &r, const Hittable &scene) {
    HitRecord rec;
    if (scene.hit(r, 0.001, infinity, rec)) {
        // map normal [-1,1] -> [0,1] for visualization
        return (rec.normal + Vec3{1, 1, 1}) * 0.5;
    }
    // background: vertical blue-white gradient
    Vec3 unit = r.dir.normalized();
    double blend = 0.5 * (unit.y + 1.0);
    return Vec3{1, 1, 1} * (1.0 - blend) + Vec3{0.5, 0.7, 1.0} * blend;
}

int main() {
    // Image
    const double aspectRatio = 16.0 / 9.0;
    const int imageWidth = 400;
    const int imageHeight = static_cast<int>(imageWidth / aspectRatio);

    // Scene
    HittableList scene;
    scene.add(new Sphere({0, 0, -1}, 0.5)); // center sphere
    scene.add(new Sphere({0, -100.5, -1}, 100)); // ground

    // Camera
    double viewportHeight = 2.0;
    double viewportWidth = aspectRatio * viewportHeight;
    double focalLength = 1.0;

    Point3 origin = {0, 0, 0};
    Vec3 horizontal = {viewportWidth, 0, 0};
    Vec3 vertical = {0, viewportHeight, 0};
    Point3 lowerLeft = origin - horizontal / 2 - vertical / 2 - Vec3{0, 0, focalLength};

    // Render
    std::ofstream out("image.ppm");
    out << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

    for (int j = imageHeight - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < imageWidth; ++i) {
            double u = double(i) / (imageWidth - 1);
            double v = double(j) / (imageHeight - 1);
            Ray r{origin, lowerLeft + horizontal * u + vertical * v - origin};
            writeColor(out, rayColor(r, scene));
        }
    }
    std::cerr << "\nDone. Saved to image.ppm\n";
    return 0;
}
