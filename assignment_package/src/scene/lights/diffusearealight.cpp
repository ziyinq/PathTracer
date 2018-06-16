#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    if (twoSided)
    {
        return emittedLight;
    }
    else
    {
        // if _w_ in the same direction of _isect_
        if (glm::dot(isect.normalGeometric, w) > 0)
        {
            return emittedLight;
        }
        else
        {
            return Color3f(0.f);
        }
    }
}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf, VisibilityTester *vis) const
{
    *pdf = 0.f;
    Intersection pShape = shape->Sample(ref, xi, pdf);
    // TODO: maybe add medium interface here? p714
    if (*pdf == 0.f || glm::length(pShape.point - ref.point) < 0.001f)
    {
        return Color3f(0.f);
    }
    *wi = glm::normalize(pShape.point - ref.point);
    *vis = VisibilityTester(ref, pShape);
    return L(pShape, -1.f * (*wi));
}

Photon DiffuseAreaLight::Sample_Photon(std::shared_ptr<Sampler> sampler, int numPhotons) const
{
    Photon thisPhoton(Point3f(0.f), Color3f(0.f), Vector3f(0.f));
    float pdfDir;
    shape->Sample_Photon(thisPhoton, sampler, pdfDir);

    // set power of photons
    // TODO: for now it is just treated as one-side!
    thisPhoton.color = emittedLight * area / (numPhotons * pdfDir);
    // thisPhoton.color = emittedLight * area * Pi/ (float)numPhotons;
    return thisPhoton;
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return shape->Pdf(ref, wi);
}
