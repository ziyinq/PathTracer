#include "lambertbtdf.h"
#include <warpfunctions.h>

Color3f LambertBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return R * InvPi;
}

Color3f LambertBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    // cosine-weighted sample the hemisphere, flipping the direction if necessary
    *wi = WarpFunctions::squareToHemisphereCosine(u);
    if (wo.z > 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float LambertBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? 0.f : AbsCosTheta(wi)*InvPi;
}
