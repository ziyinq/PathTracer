#include "volpathintegrator.h"

Color3f VolPathIntegrator::Li(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f L(0.f);
    Color3f beta(1.f);
    Ray ray(r);
    bool specularBounce = false;
    bool handleMedia = true;
    for (int bounces = 0; ; ++bounces)
    {
        // intersect ray with scene and store intersection in isect
        Intersection isect;
        bool foundIntersection = scene.Intersect(ray, &isect);
        // set t for ray and wo for isect here
        isect.wo = -ray.direction;
        if (foundIntersection) ray.tMax = isect.t;
        // sample the participating medium, if present
        MediumIntersection mi;
        if (ray.medium) beta *= ray.medium->Sample(ray, sampler, &mi);
        if (IsBlack(beta)) break;

        // handle an intersection with a medium or a surface
        if (mi.IsValid()){
            if (bounces >= depth) break;

            L += beta * UniformSampleOneLight(mi, scene, sampler, handleMedia);
            Vector3f wo = -ray.direction, wi;
            mi.phase->Sample_p(wo, &wi, sampler->Get2D());
            ray = mi.SpawnRay(wi);
            specularBounce = false;
        }
        else{

            // possibly add emitted light at intersection
            if (bounces == 0 || specularBounce){
                // add emitted light at path vertex or from the environment
                if (foundIntersection){
                    L += beta * isect.Le(-ray.direction);
                }
                // TODO:may add infiniteLight here
            }

            // terminate path if ray escaped
            if (!foundIntersection || bounces >= depth) break;

            // if it hits light, do nothing
            if(isect.objectHit->GetAreaLight()){
                 break;
            }

            // Compute scattering functions and skip over medium boundaries
            isect.ProduceBSDF();
            if (!isect.bsdf) {
                ray = isect.SpawnRay(ray.direction);
                bounces--;
                continue;
            }

            // Sample illumination from lights to find attenuated path
            // contribution
            // const Distribution1D *lightDistrib =
            // lightDistribution->Lookup(isect.p);
            L += beta * UniformSampleOneLight(isect, scene, sampler, handleMedia);

            // Sample BSDF to get new path direction
            Vector3f woW = -ray.direction, wiW;
            float pdf(0.f);
            BxDFType flags;
            Color3f f = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf, BSDF_ALL, &flags);
            wiW = glm::normalize(wiW);
            if (IsBlack(f) || pdf == 0.f) break;
            beta *= f * AbsDot(wiW, isect.normalGeometric) / pdf;
            specularBounce = ((flags & BSDF_SPECULAR) != 0);
            ray = isect.SpawnRay(wiW);

            // Account for attenuated subsurface scattering, if applicable
        }

        // possibly terminate the path with Russian roulette
        if (bounces > 3){
            float maxComp = maxComponent(beta);
            float q = std::max(0.05f, 1.f - maxComp);
            if (sampler->Get1D() < q) break;
            beta /= 1 - q;
        }
    }
    return L;
}
