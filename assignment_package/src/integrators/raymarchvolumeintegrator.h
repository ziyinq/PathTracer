#pragma once

#include "integrator.h"

class RaymarchVolumeIntegrator : public Integrator
{
public:
    RaymarchVolumeIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit, std::shared_ptr<DensityGrid> grid)
        : Integrator(bounds, s, sampler, recursionLimit), grid(grid)
    {}

    virtual Color3f Li(const Ray& ray, const Scene& scene, std::shared_ptr<Sampler> sampler, int depth) const;

    std::shared_ptr<DensityGrid> grid;
};
