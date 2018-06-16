#include "orennayarbrdf.h"
#include <warpfunctions.h>

Color3f OrenNayarBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    Float sinThetaI = SinTheta(wi);
    Float sinThetaO = SinTheta(wo);
    // Compute cosine term of Oren-Nayar model
    Float maxCos = 0;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4){
        Float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
        Float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
        Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max((Float)0, dCos);
    }
    // Compute sine and tangent terms of Oren-Nayar model
    Float sinAlpha , tanBeta;
    if (AbsCosTheta(wi) > AbsCosTheta(wo)){
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / AbsCosTheta(wi);
    }
    else{
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / AbsCosTheta(wo);
    }
    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}
