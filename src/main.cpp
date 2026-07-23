#include "main.h"

#include <atomic>
#include <fstream>
#include <limits>
#include <chrono>
#include <mutex>
#include <thread>

#include "bvh.h"
#include "dielectric.h"
#include "hittable_list.h"
#include "lambertian.h"
#include "metal.h"
#include "plane.h"
#include "sphere.h"

const double infinity = std::numeric_limits<double>::infinity();

Color rayColor(const Ray &r, const Hittable &scene, double maxBounces) {
    if (maxBounces <= 0) return {0, 0, 0};
    HitRecord rec;
    if (scene.hit(r, 0.001, infinity, rec)) {
        Ray scattered;
        Color attenuation;
        // map normal [-1,1] -> [0,1] for visualization
        if (rec.material->scatter(r, rec, attenuation, scattered)) {
            return attenuation * rayColor(scattered, scene, maxBounces - 1);
        }
        return {1,1,1};
    }
    // background: vertical blue-white gradient
    Vec3 unit = r.dir.normalized();
    double blend = 0.5 * (unit.y + 1.0);
    return Vec3{1, 1, 1} * (1.0 - blend) + Vec3{0.5, 0.7, 1.0} * blend;
}

struct Tile {
    int xStart, xEnd; // [xStart, xEnd)
    int yStart, yEnd; // [yStart, yEnd)
};

void renderTile(const Tile &tile, int imageWidth, int imageHeight,
                 const Point3 &origin, const Vec3 &lowerLeft,
                 const Vec3 &horizontal, const Vec3 &vertical,
                 const Hittable &scene, int maxBounces, int numSamplesPerPixel,
                 std::vector<Color> &framebuffer) {
    for (int row = tile.yStart; row < tile.yEnd; ++row) {
        for (int col = tile.xStart; col < tile.xEnd; ++col) {
            Color pixel = {0, 0, 0};
            for (int s = 0; s < numSamplesPerPixel; ++s) {
                const double u = (static_cast<double>(col) + generateRandomOffset()) / (imageWidth - 1);
                const double v = (static_cast<double>(row) + generateRandomOffset()) / (imageHeight - 1);
                Ray r{origin, (lowerLeft + horizontal * u + vertical * v - origin).normalized()};
                pixel = pixel + rayColor(r, scene, maxBounces);
            }
            framebuffer[row * imageWidth + col] = pixel / numSamplesPerPixel;
        }
    }
}

int main() {
    // Image
    const double aspectRatio = 16.0 / 9.0;
    const int imageWidth = 1600;
    const int imageHeight = static_cast<int>(imageWidth / aspectRatio);

    // Scene
    HittableList scene;
    scene.add(new Sphere({1, 0, -1}, 0.5, new Lambertian({0.5,0.5,0.5})));
    scene.add(new Sphere({0, 0, -1}, 0.5, new Metal({0.5,0.5,0.5}, 0.0))); // center sphere
    scene.add(new Sphere({-1, 0, -1}, 0.5, new Dielectric(1.5)));
    scene.add(new Plane({0, -0.5, 0}, {0, 1, 0}, new Lambertian({0.8, 0.8, 0.0}))); // ground

    BVHNode bvh(scene.objects, 0, static_cast<int>(scene.objects.size()));

    // Camera
    double viewportHeight = 2.0;
    double viewportWidth = aspectRatio * viewportHeight;
    double focalLength = 3.0;

    Point3 origin = {0, 0, 2};
    Vec3 horizontal = {viewportWidth, 0, 0};
    Vec3 vertical = {0, viewportHeight, 0};
    Point3 lowerLeft = origin - horizontal / 2 - vertical / 2 - Vec3{0, 0, focalLength};

    // Render

    std::ofstream out("image.ppm");
    std::vector<Color> framebuffer(imageWidth * imageHeight);
    out << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";
    auto start = std::chrono::high_resolution_clock::now();

    int tileSize = 32;
    std::vector<Tile> tiles;
    for (int row = 0; row < imageHeight; row += tileSize) {
        for (int col = 0; col < imageWidth; col += tileSize) {
            Tile tile;
            tile.xStart = col;
            tile.xEnd = std::min(col + tileSize, imageWidth);
            tile.yStart = row;
            tile.yEnd = std::min(row + tileSize, imageHeight);
            tiles.push_back(tile);

        }
    }

    std::atomic<size_t> nextTile{0};
    std::atomic<size_t> tilesCompleted{0};
    std::mutex printMutex;
    double numSamplesPerPixel = 100;
    double maxBounces = 10;
    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    auto worker = [&]() {
        while (true) {
            size_t index = nextTile.fetch_add(1);
            if (index >= tiles.size()) break;
            renderTile(tiles[index], imageWidth, imageHeight, origin, lowerLeft,
                horizontal, vertical, bvh, maxBounces, numSamplesPerPixel, framebuffer);

            size_t done = ++tilesCompleted;
            std::lock_guard<std::mutex> lock(printMutex);

            const int barWidth = 40;
            const double fraction = static_cast<double>(done) / static_cast<double>(tiles.size());
            const int filled = static_cast<int>(fraction * barWidth);
            const int percent = static_cast<int>(fraction * 100);

            std::cerr << "\r[";
            for (int i = 0; i < barWidth; ++i) std::cerr << (i < filled ? '#' : '-');
            std::cerr << "] " << percent << "%  " << done << " / " << tiles.size()
                       << " tiles completed   " << std::flush;
        }
    };

    std::vector<std::thread> workers;
    for (unsigned i = 0; i < numThreads; ++i) {workers.emplace_back(worker);}
    for (auto& worker : workers) {worker.join();}

    for (int row = imageHeight - 1; row >= 0; --row) {
        for (int col = 0; col < imageWidth; ++col) {
            writeColor(out, framebuffer[row * imageWidth + col]);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cerr << "\nDone. Render took " << elapsed.count() << " milliseconds.\n";
    std::cerr << "\nSaved to image.ppm\n";
    return 0;
};
