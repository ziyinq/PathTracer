#pragma once
#include "material.h"

class TestMaterial : public Material
{
public:
    TestMaterial(const Color3f &Kt,
                 const std::shared_ptr<QImage> &textureMap,
                 const std::shared_ptr<QImage> &normalMap)
        : Kt(Kt), textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;

private:
    Color3f Kt;                    // The spectral transmission of this material.
                                   // This is just the base color of the material

    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};
