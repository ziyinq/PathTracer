#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//TODO: Properly set worldToTangent and tangentToWorld
    : worldToTangent(glm::transpose(glm::mat3(isect.tangent, isect.bitangent, isect.normalGeometric))),
      tangentToWorld(glm::mat3(isect.tangent, isect.bitangent, isect.normalGeometric)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    Normal3f normal = n;
    normal = glm::normalize(normal);

    Vector3f tangent = t;
    tangent = glm::normalize(tangent);

    Vector3f bitangent = b;
    bitangent = glm::normalize(bitangent);

    tangentToWorld = glm::mat3(tangent, bitangent, normal);
    worldToTangent = glm::transpose(tangentToWorld);
}


//
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    Vector3f wi = worldToTangent * wiW;
    Vector3f wo = worldToTangent * woW;
    if (wo.z == 0) return Color3f(0.f);
    bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0;
    Color3f result(0.f);
    for (int i = 0; i < numBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags) &&
            ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
            (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
            result += bxdfs[i]->f(wo, wi);
    return result;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    // choose which BxDF to sample
    int matchingComps = BxDFsMatchingFlags(type);
    if (matchingComps == 0)
    {
        *pdf = 0;
        return Color3f(0.f);
    }
    int comp = std::min((int)std::floor(xi[0] * matchingComps), matchingComps -1);
    // get BxDF pointer for chosen component
    BxDF *bxdf = nullptr;
    int count = comp;
    for (int i = 0; i<numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0)
        {
            bxdf = bxdfs[i];
            break;
        }
    }
    // remap BxDF sample u to [0,1)^2
    Point2f uRemapped(xi[0] * matchingComps - comp, xi[1]);
    // sample chosen BxDF
    Vector3f wi;
    Vector3f wo = worldToTangent * woW;
    *pdf = 0.f;
    if (sampledType) *sampledType = bxdf->type;
    Color3f result = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
    if (*pdf == 0.f)
    {
        return Color3f(0.f);
    }
    *wiW = tangentToWorld * wi;
    // compute overall PDF with all matching BxDFs
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for (int i = 0; i < numBxDFs; ++i)
        {
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
        }
    }
    if (matchingComps > 1) *pdf /= matchingComps;
    // compute value of BSDF for sampled direction
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;
        result = Color3f(0.f);
        for (int i = 0; i < numBxDFs; ++i)
        {
            if (bxdfs[i]->MatchesFlags(type) &&
                    ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                    (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
                result += bxdfs[i]->f(wo, wi);
        }
    }
    return result;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    if (numBxDFs == 0.f) return 0.f;
    Vector3f wo = worldToTangent * woW, wi = worldToTangent * wiW;
    if (wo.z == 0) return 0.;
    Float pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < numBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    // uniform-sample the hemisphere, flipping the direction if necessary
    *wi = WarpFunctions::squareToHemisphereUniform(xi);
    if (wo.z < 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
