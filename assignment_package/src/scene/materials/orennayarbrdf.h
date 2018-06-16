#pragma once
#include "bsdf.h"

class OrenNayarBRDF : public BxDF
{
public:
    OrenNayarBRDF(const Color3f &R, Float sigma)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) {
        sigma = Pi * sigma / 180.f;
        Float sigma2 = sigma * sigma;
        A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B = 0.45f * sigma2 / (sigma2 + 0.09f);
    }

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;
private:
    const Color3f R;
    Float A, B;
};
