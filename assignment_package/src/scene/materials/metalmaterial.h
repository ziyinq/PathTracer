#pragma once
#include "material.h"

class MetalMaterial : public Material
{
public:
    MetalMaterial(const Color3f &eta,
                  const Color3f &k,
                  Float roughness)
        : eta(eta), k(k), roughness(roughness)
    {}

    void ProduceBSDF(Intersection *isect) const;

private:

    Color3f eta;
    Color3f k;
    Float roughness;
    // maybe add urough, vrough later
    /// GOLD
    /// "eta"  : [0.18, 0.49, 1.17],
    /// "k"  : [3.45, 2.22, 1.75],
    /// COPPER
    /// "eta" : [0.26, 0.80, 1.24]
    /// "k" : [3.65, 2.49, 2.29]
    /// Al
    /// "eta" : [1.35, 0.9, 0.7]
    /// "k" : [7.5, 6.2, 5.6]
    /// Fe
    /// "eta" : [2.9, 2.9, 2.67]
    /// "k" : [3.08, 2.92, 2.82]
    /// Silver
    /// "eta" : [0.16, 0.143, 0.136]
    /// "k" : [3.93, 3, 2.55]
};
