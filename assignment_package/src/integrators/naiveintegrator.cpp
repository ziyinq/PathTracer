#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f result(0.f);
    Vector3f woW = -ray.direction;
    Intersection isect;
    if (scene.Intersect(ray, &isect))
    {
        result += isect.Le(woW);
        if (depth > 0)
        {
            if (isect.objectHit->material == nullptr)
            {
                return result;
            }
            else
            {
                isect.ProduceBSDF();
                Vector3f wiW(0.f);
                float pdf = 0.f;
                Color3f f_term = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf);

                wiW = glm::normalize(wiW);

                Ray newRay = isect.SpawnRay(wiW);
                Color3f Li_color = Li(newRay, scene, sampler, --depth);
                if (pdf == 0.f)
                {
                    return result;
                }
                else
                {
                    result += f_term * Li_color * AbsDot(wiW, isect.normalGeometric) / pdf;
                }
            }
        }
        else
        {
            return result;
        }
    }
    else
    {
        return result;
    }
}
