#pragma once

#include "raytracing/intersection.h"
#include "medium.h"

// HomogeneousMedium Declarations
class HomogeneousMedium : public Medium {
  public:
    // HomogeneousMedium Public Methods
    HomogeneousMedium(const Color3f &sigma_a, const Color3f &sigma_s, Float g)
        : sigma_a(sigma_a),
          sigma_s(sigma_s),
          sigma_t(sigma_s + sigma_a),
          g(g) {}
    Color3f Tr(const Ray &ray, std::shared_ptr<Sampler> &sampler) const;
    Color3f Sample(const Ray &ray, std::shared_ptr<Sampler> &sampler, MediumIntersection *mi) const;

  private:
    // HomogeneousMedium Private Data
    const Color3f sigma_a, sigma_s, sigma_t;
    const Float g;
};
