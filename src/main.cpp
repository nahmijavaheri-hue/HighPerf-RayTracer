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
#include "window.h"

const double infinity = std::numeric_limits<double>::infinity();

Color rayColor(Ray r, const Hittable &scene, int maxBounces) {
    Color throughput = {1, 1, 1};

    for (int depth = 0; depth < maxBounces; ++depth) {
        HitRecord rec;
        if (!scene.hit(r, 0.001, infinity, rec)) {
            // background: vertical blue-white gradient
            Vec3 unit = r.dir.normalized();
            double blend = 0.5 * (unit.y + 1.0);
            Color background = Vec3{1, 1, 1} * (1.0 - blend) + Vec3{0.5, 0.7, 1.0} * blend;
            return throughput * background;
        }

        Ray scattered;
        Color attenuation;
        if (!rec.material->scatter(r, rec, attenuation, scattered)) {
            return throughput;
        }

        throughput = throughput * attenuation;
        r = scattered;
    }

    return {0, 0, 0};
}

class SceneWithGround : public Hittable {
public:
    const Hittable &bvh;
    const Hittable &groundPlane;

    SceneWithGround(const Hittable &bvh, const Hittable &groundPlane)
        : bvh(bvh), groundPlane(groundPlane) {}

    bool hit(const Ray &r, double tMin, double tMax, HitRecord &rec) const override {
        bool hitBvh = bvh.hit(r, tMin, tMax, rec);
        bool hitGround = groundPlane.hit(r, tMin, hitBvh ? rec.t : tMax, rec);
        return hitBvh || hitGround;
    }

    AABB boundingBox() const override {
        return bvh.boundingBox(); // never queried at the top level, just satisfies the interface
    }
};

struct Tile {
    int xStart, xEnd; // [xStart, xEnd)
    int yStart, yEnd; // [yStart, yEnd)
};

void renderTile(const Tile &tile, int imageWidth, int imageHeight,
                 const Point3 &origin, const Vec3 &lowerLeft,
                 const Vec3 &horizontal, const Vec3 &vertical,
                 const Hittable &scene, int maxBounces, int numSamplesPerPixel,
                 std::vector<Color> &framebuffer) {
    const int tileWidth = tile.xEnd - tile.xStart;
    const int tileHeight = tile.yEnd - tile.yStart;
    std::vector<Color> accum(tileWidth * tileHeight, Color{0, 0, 0});

    for (int s = 0; s < numSamplesPerPixel; ++s) {
        for (int row = tile.yStart; row < tile.yEnd; ++row) {
            for (int col = tile.xStart; col < tile.xEnd; ++col) {
                const double u = (static_cast<double>(col) + generateRandomOffset()) / (imageWidth - 1);
                const double v = (static_cast<double>(row) + generateRandomOffset()) / (imageHeight - 1);
                Ray r{origin, (lowerLeft + horizontal * u + vertical * v - origin).normalized()};

                const int localIdx = (row - tile.yStart) * tileWidth + (col - tile.xStart);
                accum[localIdx] = accum[localIdx] + rayColor(r, scene, maxBounces);
                framebuffer[row * imageWidth + col] = accum[localIdx] / (s + 1);
            }
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Scene geometry doesn't depend on any render setting (resolution/samples/
    // bounces), so it's built once and reused across every render.
    HittableList scene;
    scene.add(new Sphere({1, 0, -1}, 0.5, new Lambertian({0.5,0.5,0.5})));
    scene.add(new Sphere({0, 0, -1}, 0.5, new Metal({0.5,0.5,0.5}, 0.0))); // center sphere
    scene.add(new Sphere({-1, 0, -1}, 0.5, new Dielectric(1.5)));
    // ground plane is kept out of the BVH — its AABB is effectively infinite (see plane.h),
    // which would make the tree's box tests useless as culling. Tested separately instead.

    BVHNode bvh(scene.objects, 0, static_cast<int>(scene.objects.size()));
    Plane groundPlane({0, -0.5, 0}, {0, 1, 0}, new Lambertian({0.8, 0.8, 0.0}));
    SceneWithGround world(bvh, groundPlane);

    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    // persistent, reused/reset on every render rather than recreated —
    // the orchestrator thread captures these by reference, so they need to
    // outlive any single render
    std::mutex printMutex;
    std::atomic<size_t> nextTile{0};
    std::atomic<size_t> tilesCompleted{0};
    std::vector<Tile> tiles;
    std::vector<Color> framebuffer;
    std::ofstream out;
    std::thread orchestrator;

    SettingsWindow settingsWindow(320, 260);
    PreviewWindow previewWindow(1600, 900); // blank until the first render starts

    auto startRender = [&](const RenderSettings &settings) {
        // previous render already signaled completion (button re-enabled) before
        // this can be reached, so this join returns immediately — required
        // cleanup before reassigning a new thread into `orchestrator`
        if (orchestrator.joinable()) orchestrator.join();

        const int imageWidth = settings.imageWidth;
        const int imageHeight = settings.imageHeight;
        const int maxBounces = settings.maxBounces;
        const int numSamplesPerPixel = settings.numSamplesPerPixel;

        const double aspectRatio = static_cast<double>(imageWidth) / imageHeight;
        const double viewportHeight = 2.0;
        const double viewportWidth = aspectRatio * viewportHeight;
        const double focalLength = 3.0;
        const Point3 origin = {0, 0, 2};
        const Vec3 horizontal = {viewportWidth, 0, 0};
        const Vec3 vertical = {0, viewportHeight, 0};
        const Point3 lowerLeft = origin - horizontal / 2 - vertical / 2 - Vec3{0, 0, focalLength};

        previewWindow.reset(imageWidth, imageHeight);

        out.close();
        out.clear();
        out.open("image.ppm");
        out << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

        framebuffer.assign(static_cast<size_t>(imageWidth) * imageHeight, Color{0, 0, 0});

        const int tileSize = 32;
        tiles.clear();
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
        nextTile.store(0);
        tilesCompleted.store(0);

        auto start = std::chrono::high_resolution_clock::now();
        previewWindow.attachRenderState(&framebuffer, &tilesCompleted, tiles.size(), start);
        previewWindow.startLiveUpdates();
        settingsWindow.setRenderButtonEnabled(false);

        orchestrator = std::thread([&, imageWidth, imageHeight, origin, lowerLeft, horizontal, vertical,
                                       maxBounces, numSamplesPerPixel, start]() {
            // worker is defined here, not in startRender's scope, because this
            // lambda (and everything it captures by value above) has to survive
            // after startRender() itself returns
            auto worker = [&]() {
                while (true) {
                    size_t index = nextTile.fetch_add(1);
                    if (index >= tiles.size()) break;
                    renderTile(tiles[index], imageWidth, imageHeight, origin, lowerLeft,
                        horizontal, vertical, world, maxBounces, numSamplesPerPixel, framebuffer);

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
            for (auto& w : workers) {w.join();}

            for (int row = imageHeight - 1; row >= 0; --row) {
                for (int col = 0; col < imageWidth; ++col) {
                    writeColor(out, framebuffer[row * imageWidth + col]);
                }
            }
            out.flush();

            // signal both windows only after the file is actually fully written,
            // since the "done" message displayed claims exactly that
            PostMessage(previewWindow.getHwnd(), WM_APP + 1, 0, 0);
            PostMessage(settingsWindow.getHwnd(), WM_APP + 1, 0, 0);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cerr << "\nDone. Render took " << elapsed.count() << " milliseconds.\n";
            std::cerr << "\nSaved to image.ppm\n";
        });
    };

    settingsWindow.onRenderClicked = [&]() { startRender(settingsWindow.getSettings()); };

    previewWindow.show();
    settingsWindow.show();
    BringWindowToTop(settingsWindow.getHwnd());
    SetForegroundWindow(settingsWindow.getHwnd());

    // one message loop services both windows — Win32 dispatches by HWND, so a
    // single thread's loop handles every window that thread created
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (orchestrator.joinable()) orchestrator.join(); // required: a joinable std::thread destroyed without join()/detach() calls std::terminate()
    return static_cast<int>(msg.wParam);
};
