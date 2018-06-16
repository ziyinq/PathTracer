#pragma once

#include "light.h"

class SpotLight : public Light
{
public:
    // SpotLight Public Methods
    SpotLight(const Transform &t, const Color3f &I,
              Float totalWidth, Float falloffStart);

    Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis = nullptr) const;
    Float Falloff(const Vector3f &w) const;
    Color3f Power() const;
    Float Pdf_Li(const Intersection &, const Vector3f &) const;
//    Color3f Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
//                       Ray *ray, Normal3f *nLight, Float *pdfPos,
//                       Float *pdfDir) const;
//    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
//                Float *pdfDir) const;
    virtual Photon Sample_Photon(std::shared_ptr<Sampler> sampler, int numPhotons) const
    {}

private:
    // SpotLight Private Data
    const Point3f pLight;
    const Color3f I;
    const Float cosTotalWidth, cosFalloffStart;
};
