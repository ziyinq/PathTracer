#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    return Color3f(FrDielectric(cosThetaI, etaI, etaT));
}

Color3f FresnelConductor::Evaluate(Float cosThetaI) const {
    return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
}
