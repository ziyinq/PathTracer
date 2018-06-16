#pragma once
#include "bsdf.h"
#include "fresnel.h"
#include "microfacet.h"

class MicrofacetBTDF : public BxDF
{
public:
    MicrofacetBTDF(const Color3f &T, Float etaA, Float etaB, MicrofacetDistribution* distribution, Fresnel* fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)), T(T), distribution(distribution), fresnel(fresnel), etaA(etaA), etaB(etaB) {}

    virtual ~MicrofacetBTDF(){delete fresnel; delete distribution;}

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &xi, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;


  private:
    const Color3f T; // The energy scattering coefficient of this BRDF (i.e. its color)
    const MicrofacetDistribution* distribution;
    const Fresnel* fresnel; // A class that will determine the reflectivity coefficient at this point
    const Float etaA;
    const Float etaB;
};
