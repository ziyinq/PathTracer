#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f LdResult(0.f);
    Color3f LnResult(0.f);
    Color3f result(0.f);
    Vector3f woW = -ray.direction;
    Intersection isect;
    VisibilityTester vis;
    if (scene.Intersect(ray, &isect))
    {
        result += isect.Le(woW);
        if (isect.objectHit->material == nullptr)
        {
            return result;
        }
        else
        {
            isect.ProduceBSDF();

            // evaluate light PDF sampling part
            Vector3f LdwiW(0.f);
            float Ld_lightPdf = 0.f;
            // randomly select a light source and call its Sample_Li function
            int lightNum = scene.lights.size();
            int randomLight = std::min((int)std::floor(sampler->Get1D() * lightNum), lightNum -1);
            Color3f Ld_color = scene.lights[randomLight]->Sample_Li(isect, sampler->Get2D(), &LdwiW, &Ld_lightPdf, &vis);
            Color3f Ld_f_term = isect.bsdf->f(woW, LdwiW);
            // generate shadowRay and do shadow test
            Ray shadowRay = isect.SpawnRay(LdwiW);
            Intersection shadowIntersect;
            // special case for Ld part
            if (scene.Intersect(shadowRay, &shadowIntersect))
            {
                if (shadowIntersect.objectHit->GetAreaLight() == scene.lights[randomLight].get())
                {
                    if (Ld_lightPdf != 0.f)
                    {
                        float Ld_brdfPdf = isect.bsdf->Pdf(woW, LdwiW);
                        Ld_lightPdf = Ld_lightPdf / lightNum;
                        float weight = PowerHeuristic(1, Ld_lightPdf, 1, Ld_brdfPdf);
                        //float weight = BalanceHeuristic(1, Ld_lightPdf, 1, Ld_brdfPdf);
                        LdResult += weight * Ld_f_term * Ld_color * AbsDot(LdwiW, isect.normalGeometric) / Ld_lightPdf;
                    }
                }
            }

            // evaluate BRDF PDF sampling part
            Vector3f LnwiW(0.f);
            float Ln_brdfPdf = 0.f;
            Color3f Ln_color(0.f);
            Color3f Ln_f_term = isect.bsdf->Sample_f(woW, &LnwiW, sampler->Get2D(), &Ln_brdfPdf);
            LnwiW = glm::normalize(LnwiW);
            Ray newRay = isect.SpawnRay(LnwiW);
            Intersection rayIntersect;
            float Ln_lightPdf = scene.lights[randomLight]->Pdf_Li(isect, LnwiW);
            Ln_lightPdf = Ln_lightPdf / lightNum;
            // special case for Ln part
            if (Ln_brdfPdf > 0.f)
            {
                float weight = 1.f;
                // if PDF of light is nonzero given w_j
                if (Ln_lightPdf > 0.f)
                {
                    weight = PowerHeuristic(1, Ln_brdfPdf, 1, Ln_lightPdf);
                    //weight = BalanceHeuristic(1, Ln_brdfPdf, 1, Ln_lightPdf);
                    if (scene.Intersect(newRay, &rayIntersect))
                    {
                        if (rayIntersect.objectHit->GetAreaLight() == scene.lights[randomLight].get())
                        {
                            Ln_color = rayIntersect.Le(-LnwiW);
                        }
                    }
                }
                LnResult = weight * Ln_f_term * Ln_color * AbsDot(LnwiW, isect.normalGeometric) / Ln_brdfPdf;
            }
            result += LdResult + LnResult;
            return result;
        }
    }
    else
    {
        return result;
    }
}


