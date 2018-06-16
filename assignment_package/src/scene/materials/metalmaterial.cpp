#include "metalmaterial.h"
#include "fresnel.h"
#include "microfacet.h"
#include "microfacetbrdf.h"

void MetalMaterial::ProduceBSDF(Intersection *isect) const
{
    // Important! Must initialize the intersection's BSDF!
    isect->bsdf = std::make_shared<BSDF>(*isect);

    //Convert roughness from 0,1 scale to alpha term in Trowbridge-Reitz distribution
    float rough = roughness;
    //rough = RoughnessToAlpha(rough);
    MicrofacetDistribution* distrib = new TrowbridgeReitzDistribution(rough, rough);

    isect->bsdf->Add(new MicrofacetBRDF(Color3f(1.f), distrib, new FresnelConductor(Color3f(1.f), eta, k)));
}
