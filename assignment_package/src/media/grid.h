#pragma once

#include "media/medium.h"
#include "scene/transform.h"

// TODO: for heterogeneous
//// GridDensityMedium Declarations
//class GridDensityMedium : public Medium {
//  public:
//    // GridDensityMedium Public Methods
//    GridDensityMedium(const Color3f &sigma_a, const Color3f &sigma_s, Float g,
//                      int nx, int ny, int nz, const Transform &mediumToWorld,
//                      const Float *d)
//        : sigma_a(sigma_a),
//          sigma_s(sigma_s),
//          g(g),
//          nx(nx),
//          ny(ny),
//          nz(nz),
//          WorldToMedium(Inverse(mediumToWorld)),
//          density(new Float[nx * ny * nz]) {
//        densityBytes += nx * ny * nz * sizeof(Float);
//        memcpy((Float *)density.get(), d, sizeof(Float) * nx * ny * nz);
//        // Precompute values for Monte Carlo sampling of _GridDensityMedium_
//        sigma_t = (sigma_a + sigma_s)[0];
//        if (Color3f(sigma_t) != sigma_a + sigma_s)
//            Error(
//                "GridDensityMedium requires a spectrally uniform attenuation "
//                "coefficient!");
//        Float maxDensity = 0;
//        for (int i = 0; i < nx * ny * nz; ++i)
//            maxDensity = std::max(maxDensity, density[i]);
//        invMaxDensity = 1 / maxDensity;
//    }

//    Float Density(const Point3f &p) const;
//    Float D(const Point3i &p) const {
//        Bounds3i sampleBounds(Point3i(0, 0, 0), Point3i(nx, ny, nz));
//        if (!InsideExclusive(p, sampleBounds)) return 0;
//        return density[(p.z * ny + p.y) * nx + p.x];
//    }
//    Color3f Sample(const Ray &ray, Sampler &sampler, MemoryArena &arena,
//                    MediumInteraction *mi) const;
//    Color3f Tr(const Ray &ray, Sampler &sampler) const;

//  private:
//    // GridDensityMedium Private Data
//    const Color3f sigma_a, sigma_s;
//    const Float g;
//    const int nx, ny, nz;
//    const Transform WorldToMedium;
//    std::unique_ptr<Float[]> density;
//    Float sigma_t;
//    Float invMaxDensity;
//};
