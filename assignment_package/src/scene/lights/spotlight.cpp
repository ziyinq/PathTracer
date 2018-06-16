#include "spotlight.h"

// SpotLight Method Definitions
SpotLight::SpotLight(const Transform &t,
                     const Color3f &I,
                     Float totalWidth, Float falloffStart)
    : Light(t),
      pLight(Point3f(t.T()*glm::vec4(0, 0, 0, 1.f))),
      I(I),
      cosTotalWidth(std::cos(Pi *totalWidth / 180.f)),
      cosFalloffStart(std::cos(Pi * falloffStart / 180.f))
    {}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf,
                              VisibilityTester *vis) const {
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    Intersection isect;
    isect.point = pLight;
    *vis = VisibilityTester(ref, isect);
    return I * Falloff(-*wi) / glm::length2(pLight - ref.point);
}

Float SpotLight::Falloff(const Vector3f &w) const {
    Vector3f wl = glm::normalize(transform.rotateT() * w);
    Float cosTheta = -wl.y;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;
    // Compute falloff inside spotlight cone
    Float delta =
        (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

Color3f SpotLight::Power() const {
    return I * 2.f * Pi * (1.f - 0.5f * (cosFalloffStart + cosTotalWidth));
}

Float SpotLight::Pdf_Li(const Intersection &, const Vector3f &) const {
    return 0.f;
}
