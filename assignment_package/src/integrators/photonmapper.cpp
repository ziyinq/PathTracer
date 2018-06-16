#include "photonmapper.h"

PhotonMapper::PhotonMapper(int numPhotons, std::vector<Photon> *photons, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit)
    : Integrator(Bounds2i(Point2i(0,0), Point2i(0,0)), s, sampler, recursionLimit), preprocessing(true), numPhotons(numPhotons), photons(photons)
{}

PhotonMapper::PhotonMapper(Bounds2i bounds, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, float searchR)
    : Integrator(bounds, s, sampler, recursionLimit), preprocessing(false), numPhotons(0), photons(nullptr), kdtree(kdtree), searchR(searchR)
{}

void PhotonMapper::Render()
{
    // PhotonMapper's Render() function has
    // two modes: when preprocessing, it traces
    // a collection of photons through the scene
    // and stores them in the given k-d tree.
    // If not preprocessing, it runs like a regular Integrator
    // and invokes Li().
    if(preprocessing)
    {
        // TODO
        // Determine how many photons to assign to each light source
        // given numPhotons and the intensity of each light.
        // Shoot a number of photons equal to numPhotons from
        // the lights, bouncing them through the scene and pushing
        // back the result of each bounce to the photons vector
        // stored in the PhotonMapper.
        int photonEachLight = numPhotons / scene->lights.size();
        for (auto l : scene->lights)
        {
            for(int i = 0; i < photonEachLight; i++)
            {
                // 1. generate a set of photons for each light
                int totalPhotons = 8 * photonEachLight;
                Photon thisPhoton = l->Sample_Photon(sampler, totalPhotons);
                Ray ray(thisPhoton.pos + 0.00001f*thisPhoton.wi, thisPhoton.wi);
                for (int j = 0; j < recursionLimit; j++)
                {
                    // conceptually woW and wiW are reversed in this process!
                    Color3f throughput(1.f);
                    // reverse the incoming photon direction
                    Vector3f woW = -ray.direction;
                    Intersection isect;
                    if (scene->Intersect(ray, &isect))
                    {
                        if (isect.objectHit->material != nullptr)
                        {
                            isect.ProduceBSDF();
                            Vector3f wiW(0.f);
                            float pdf = 0.f;
                            BxDFType flags;
                            Color3f f = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf, BSDF_ALL, &flags);
                            wiW = glm::normalize(wiW);
                            if (IsBlack(f) || pdf == 0.f) break;

                            // change photon position
                            thisPhoton.pos = isect.point;
                            // TODO: store incoming light or outgoing light?
                            // TODO: do we store the first hit?
                            // store photon if non-specular and not first hit
                            if ((flags & BSDF_SPECULAR) != 1 && j > 0)
                            {
                                photons->push_back(thisPhoton);
                            }
                            // change photon outgoing direction and color
                            thisPhoton.wi = wiW;
                            thisPhoton.color *= f * AbsDot(wiW, isect.normalGeometric) / pdf;

                            throughput *= f * AbsDot(wiW, isect.normalGeometric) / pdf;
                            ray = isect.SpawnRay(wiW);
                            // possibly terminate the path with Russian roulette
                            if (j > 3)
                            {
                                float maxComp = maxComponent(throughput);
                                float q = std::max(0.05f, 1.f - maxComp);
                                if (sampler->Get1D() < q) break;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        Integrator::Render(); // Invokes Li for each ray from a pixel
    }
}

Color3f PhotonMapper::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f result(0.f);
    Vector3f woW = -ray.direction;
    Intersection isect;
    if (scene.Intersect(ray, &isect))
    {
        if (isect.objectHit->material == nullptr)
        {
            result += isect.Le(woW);
        }
        else
        {
            isect.ProduceBSDF();

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
                Color3f Ld_color = scene.lights[randomLight]->Sample_Li(isect, sampler->Get2D(), &LdwiW, &Ld_lightPdf);
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
                result += (LdResult + LnResult) * (float)lightNum;
            }

            /// Global Illumination part
            /// TODO: search photons directly or cast ray?
            float rangeArea = Pi * searchR * searchR;
            std::vector<Photon> rangePhotons = kdtree->particlesInSphere(isect.point, searchR);
            Color3f photonResult(0.f);
            for (auto p : rangePhotons)
            {
                float pdf = isect.bsdf->Pdf(woW, -p.wi);
                Color3f pf = isect.bsdf->f(woW, -p.wi);
                if (pdf == 0.f) continue;
                if (AbsDot(isect.normalGeometric, p.pos - isect.point) < 0.0001)
                    // TODO: do we have to divide it by pdf?
                    photonResult += pf * p.color * AbsDot(isect.normalGeometric, p.wi) / pdf;
            }
            result += photonResult / rangeArea;
        }
    }
    return result;
}
