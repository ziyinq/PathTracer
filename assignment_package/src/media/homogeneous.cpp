#include "homogeneous.h"

Color3f HomogeneousMedium::Tr(const Ray &ray, std::shared_ptr<Sampler> &sampler) const {
    return Exp(-sigma_t * std::min(ray.tMax * glm::length(ray.direction), MaxFloat));
//    return Exp(-sigma_t * std::min(ray.tMax * ray.length, MaxFloat));
}

Color3f HomogeneousMedium::Sample(const Ray &ray, std::shared_ptr<Sampler> &sampler,
                                   MediumIntersection *mi) const {
    // Sample a channel and distance along the ray
    int channel = std::min((int)(sampler->Get1D() * 3), 2);
    Float dist = -std::log(1 - sampler->Get1D()) / sigma_t[channel];
    Float t = std::min(dist / glm::length(ray.direction), ray.tMax);
//    Float t = std::min(dist / ray.length, ray.tMax);
    bool sampledMedium = t < ray.tMax;
    HenyeyGreenstein *phase = new HenyeyGreenstein(g);
    if (sampledMedium)
        *mi = MediumIntersection(ray(t), -ray.direction, ray.time, this, phase);

    // Compute the transmittance and sampling density
    Color3f Tr = Exp(-sigma_t * std::min(t, MaxFloat) * glm::length(ray.direction));
//    Color3f Tr = Exp(-sigma_t * std::min(t, MaxFloat) * ray.length);

    // Return weighting factor for scattering from homogeneous medium
    Color3f density = sampledMedium ? (sigma_t * Tr) : Tr;
    Float pdf = 0;
    for (int i = 0; i < 3; ++i) pdf += density[i];
    pdf *= 1.f / 3.f;
    // ????
    if (pdf == 0) {
        if (IsBlack(Tr))  pdf = 1.f;
    }
    return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}
