#pragma once

#include "samplers/sampler.h"
#include "globals.h"
#include "raytracing/ray.h"
//#include "raytracing/intersection.h"

class MediumIntersection;

class PhaseFunction{
public:
    virtual ~PhaseFunction() {}
    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual Float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const = 0;
};

inline Float PhaseHG(Float cosTheta, Float g){
    Float denom = 1.f + g * g + 2 * g * cosTheta;
    return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}

class Medium{
public:
    virtual ~Medium() {}
    virtual Color3f Tr(const Ray &ray, std::shared_ptr<Sampler> &Sampler) const = 0;
    virtual Color3f Sample(const Ray &ray, std::shared_ptr<Sampler> &sampler, MediumIntersection *mi) const = 0;
};

class HenyeyGreenstein : public PhaseFunction{
public:
    HenyeyGreenstein(Float g) : g(g) {}
    Float p(const Vector3f &wo, const Vector3f &wi) const;
    Float Sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &sample) const;
private:
    const Float g;
};

class MediumInterface {
public:
    MediumInterface() : inside(nullptr), outside(nullptr) {}
    // MediumInterface Public Methods
    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {}
    MediumInterface(const Medium *inside, const Medium *outside)
        : inside(inside), outside(outside) {}
    bool IsMediumTransition() const { return inside != outside; }
    const Medium *inside, *outside;
};

inline Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi,
                                   const Vector3f &x, const Vector3f &y,
                                   const Vector3f &z) {
    return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y +
           cosTheta * z;
}
