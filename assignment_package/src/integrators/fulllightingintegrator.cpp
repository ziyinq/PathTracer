#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f result(0.f);
    Color3f beta(1.f);
    Ray ray = r;
    Vector3f woW;
    bool specularBounce = false;
    VisibilityTester vis;
    // TODO: implement etaScale here?
    for(int bounces = 0;; ++bounces){
        // intersect ray with scene and store intersection in isect
        Intersection isect;
        bool foundIntersection = scene.Intersect(ray, &isect);
        woW = -ray.direction;

        // possibly add emitted light at intersection
        if (bounces == 0 || specularBounce){
            // add emitted light at path vertex or from the environment
            if (foundIntersection){
                result += beta * isect.Le(woW);
            }
            // TODO:may add infiniteLight here
        }

        // terminate path if ray escaped
        if (!foundIntersection || bounces >= depth) break;

        // if it hits light, do nothing
        if(isect.objectHit->GetAreaLight()){
             break;
        }

        // compute scattering functions and skip over medium boudaries
        isect.ProduceBSDF();

        // TODO: skip over medium boudnaries?

        /// direct lighting part
        // evaluate light PDF sampling part, ignored with specular materials
        if (isect.bsdf->BxDFsMatchingFlags(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0){
            Color3f LdResult(0.f);
            Color3f LnResult(0.f);
            Vector3f LdwiW(0.f);
            float Ld_lightPdf = 0.f;
            // randomly select a light source and call its Sample_Li function
            int lightNum = scene.lights.size();
            if (lightNum == 0) return Color3f(0.f);
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
                        float weight = PowerHeuristic(1, Ld_lightPdf, 1, Ld_brdfPdf);
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
            // special case for Ln part
            if (Ln_brdfPdf > 0.f)
            {
                float weight = 1.f;
                // if PDF of light is nonzero given w_j
                if (Ln_lightPdf > 0.f)
                {
                    weight = PowerHeuristic(1, Ln_brdfPdf, 1, Ln_lightPdf);
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
            result += beta * (LdResult + LnResult) * (float)lightNum;
        }

        // Sample BSDF to get new path direction
        Vector3f wiW(0.f);
        float pdf(0.f);
        BxDFType flags;
        Color3f f = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf, BSDF_ALL, &flags);
        wiW = glm::normalize(wiW);
        if (IsBlack(f) || pdf == 0.f) break;
        beta *= f * AbsDot(wiW, isect.normalGeometric) / pdf;
        specularBounce = ((flags & BSDF_SPECULAR) != 0);
        ray = isect.SpawnRay(wiW);

        // TODO: account for subsurface scattering if applicable

        // possibly terminate the path with Russian roulette
        if (bounces > 3){
            float maxComp = maxComponent(beta);
            float q = std::max(0.05f, 1.f - maxComp);
            if (sampler->Get1D() < q) break;
            beta /= 1 - q;
        }
    }
    return result;
}
