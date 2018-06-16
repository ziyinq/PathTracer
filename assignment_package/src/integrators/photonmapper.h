#pragma once
#include "integrator.h"
#include <scene/photon.h>
#include "scene/kdtree.h"

class PhotonMapper : public Integrator
{
public:
    PhotonMapper(int numPhotons, std::vector<Photon>* photons, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit);
    PhotonMapper(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, float searchR);

    virtual void Render();

    virtual Color3f Li(const Ray& ray, const Scene& scene, std::shared_ptr<Sampler> sampler, int depth) const;

private:
    bool preprocessing;
    //Preprocess variables
    int numPhotons;
    std::vector<Photon>* photons;

    float searchR;
    KDTree *kdtree;
};

