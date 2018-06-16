#pragma once
#include <globals.h>

// Pronounced fruh-nel, NOT fress-nel
// It's the last name of Augustin-Jean Fresnel, a French physicist who
// studied the theory of wave optics

// An abstract class for computing Fresnel reflection coefficients
// This simply means "how much should we weight the reflected color
// of this surface compared to its other BxDF colors?"
class Fresnel {
 public:
   // Fresnel Interface
   virtual ~Fresnel(){}
   virtual Color3f Evaluate(float cosI) const = 0;
};

// A Fresnel class that always returns 100% reflection regardless of ray direction
// While not completely physically accurate, it is useful for testing BRDF
// implementations
class FresnelNoOp : public Fresnel {
  public:
    Color3f Evaluate(float) const { return Color3f(1.); }
};

class FresnelNoReflect : public Fresnel {
public:
    Color3f Evaluate(float) const { return Color3f(0.f); }
};

// A Fresnel class that computes the Fresnel reflection coefficient for
// dielectric surfaces such as glass, water, or air.
class FresnelDielectric : public Fresnel {
  public:
    FresnelDielectric(float etaI, float etaT) : etaI(etaI), etaT(etaT) {}
    Color3f Evaluate(float cosThetaI) const;

  private:
    Float etaI, etaT; // The index of refraction of the medium through which the incident ray
                      // has traveled, and the index of refraction of the medium into which
                      // the incident ray will be transmitted and refracted.
};

class FresnelConductor : public Fresnel {
  public:
    // FresnelConductor Public Methods
    FresnelConductor(const Color3f &etaI, const Color3f &etaT,
                     const Color3f &k)
        : etaI(etaI), etaT(etaT), k(k) {}
    Color3f Evaluate(Float cosThetaI) const;

  private:
    Color3f etaI, etaT, k;
};


inline float FrDielectric(float cosThetaI, float etaI, float etaT)
{
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);

    // potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering)
    {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // compute cosThetaT using Snell's law
    float sinThetaI = std::sqrt(std::max(0.f, 1 - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    // handle totoal internal reflection
    if (sinThetaT >= 1)
        return 1.f;
    float cosThetaT = std::sqrt(std::max(0.f, 1 - sinThetaT * sinThetaT));
    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2.f;
}

inline Color3f FrConductor(Float cosThetaI, const Color3f &etai,
                     const Color3f &etat, const Color3f &k) {
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);
    Color3f eta = etat / etai;
    Color3f etak = k / etai;

    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1. - cosThetaI2;
    Color3f eta2 = eta * eta;
    Color3f etak2 = etak * etak;

    Color3f t0 = eta2 - etak2 - sinThetaI2;
    Color3f a2plusb2 = glm::sqrt(t0 * t0 + 4.f * eta2 * etak2);
    Color3f t1 = a2plusb2 + cosThetaI2;
    Color3f a = glm::sqrt(0.5f * (a2plusb2 + t0));
    Color3f t2 = (Float)2 * cosThetaI * a;
    Color3f Rs = (t1 - t2) / (t1 + t2);

    Color3f t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Color3f t4 = t2 * sinThetaI2;
    Color3f Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}
