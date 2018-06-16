#include "transmissivematerial.h"
#include "specularbtdf.h"
#include "microfacet.h"
#include "microfacetbrdf.h"
#include "microfacetbtdf.h"

void TransmissiveMaterial::ProduceBSDF(Intersection *isect) const
{
    // Important! Must initialize the intersection's BSDF!
    isect->bsdf = std::make_shared<BSDF>(*isect, indexOfRefraction);

    Color3f color = Kt;
    if(this->textureMap)
    {
        color *= Material::GetImageColor(isect->uv, this->textureMap.get());
    }
    if(this->normalMap)
    {
        isect->bsdf->normal = isect->bsdf->tangentToWorld *  Material::GetImageColor(isect->uv, this->normalMap.get());
        //Update bsdf's TBN matrices to support the new normal
        Vector3f tangent, bitangent;
        CoordinateSystem(isect->bsdf->normal, &tangent, &bitangent);
        isect->bsdf->UpdateTangentSpaceMatrices(isect->bsdf->normal, tangent, bitangent);
    }
    if (roughness == 0)
    {
        isect->bsdf->Add(new SpecularBTDF(color, 1.f, indexOfRefraction, new FresnelNoReflect()));
    }
    else{
        float rough = roughness;
        if(this->roughnessMap)
        {
            // Gets the greyscale value of the roughness map image and attenuates it by the
            // overall roughness of this material
            Color3f roughRGB = Material::GetImageColor(isect->uv, this->roughnessMap.get());
            rough *= (0.299 * roughRGB.r + 0.587 * roughRGB.g + 0.114 * roughRGB.b);
        }
        //Convert roughness from 0,1 scale to alpha term in Trowbridge-Reitz distribution
        //  rough = RoughnessToAlpha(rough);
        MicrofacetDistribution* distrib = new TrowbridgeReitzDistribution(rough, rough);
        //MicrofacetDistribution* distrib = new BeckmannDistribution(rough, rough);

        isect->bsdf->Add(new MicrofacetBTDF(color, 1.f, indexOfRefraction, distrib, new FresnelNoReflect()));
    }
}
