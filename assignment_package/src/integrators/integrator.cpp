#include "integrator.h"

void Integrator::run()
{
    Render();
}

void Integrator::Render()
{
    // Compute the bounds of our sample, clamping to screen's max bounds if necessary
    // Instantiate a FilmTile to store this thread's pixel colors
    std::vector<Point2i> tilePixels = bounds.GetPoints();
    // For every pixel in the FilmTile:
    for(Point2i pixel : tilePixels)
    {
        ///Uncomment this to debug a particular pixel within this tile
//        if(pixel.x != 200 && pixel.y != 200)
//        {
//            continue;
//        }
        Color3f pixelColor(0.f);
        // Ask our sampler for a collection of stratified samples, then raycast through each sample
        std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
        int i = 0;
        for(Point2f sample : pixelSamples)
        {
            i++;
            sample = sample + Point2f(pixel); // _sample_ is [0, 1), but it needs to be translated to the pixel's origin.
            // Generate a ray from this pixel sample
            Ray ray = camera->Raycast(sample);

            Color3f sigma_a(0.f);
            Color3f sigma_s(0.05f);
            Float g = 0.5;
            HomogeneousMedium *rayMedium= new HomogeneousMedium(sigma_a, sigma_s, g);
            ray.medium = rayMedium;

            // Get the L (energy) for the ray by calling Li(ray, scene, tileSampler, arena)
            // Li is implemented by Integrator subclasses, like DirectLightingIntegrator
            Color3f L = Li(ray, *scene, sampler, recursionLimit);
            // Accumulate color in the pixel
            pixelColor += L;
        }
        // Average all samples' energies
        pixelColor /= pixelSamples.size();
        film->SetPixelColor(pixel, glm::clamp(pixelColor, 0.f, 1.f));
    }
    //We're done here! All pixels have been given an averaged color.
}


void Integrator::ClampBounds()
{
    Point2i max = bounds.Max();
    max = Point2i(std::min(max.x, film->bounds.Max().x), std::min(max.y, film->bounds.Max().y));
    bounds = Bounds2i(bounds.Min(), max);
}

Color3f UniformSampleOneLight(const Intersection &isect,
                              const Scene &scene,
                              std::shared_ptr<Sampler> sampler,
                              bool handleMedia)
{
    // randomly choose a single light to sample
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Color3f(0.f);
    int lightNum;
    Float lightPdf;
    lightNum = std::min((int)(sampler->Get1D() * nLights), nLights - 1);
    lightPdf = Float(1) / nLights;
    const std::shared_ptr<Light> &light = scene.lights[lightNum];
    Point2f uLight = sampler->Get2D();
    Point2f uScattering = sampler->Get2D();
    return EstimateDirect(isect, uScattering, *light, uLight, scene, sampler, handleMedia)
            / lightPdf;
}

Color3f EstimateDirect(const Intersection &isect,
                       const Point2f &uScattering, const Light &light,
                       const Point2f &uLight, const Scene &scene,
                       std::shared_ptr<Sampler> sampler, bool handleMedia, bool specular)
{
    BxDFType bsdfFlags = specular ? BSDF_ALL : BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
    Color3f Ld(0.f);
    // sample light source with multiple importance sampling
    Vector3f wi;
    Float lightPdf = 0.f, scatteringPdf = 0.f;
    VisibilityTester visibility;
    Color3f Li = light.Sample_Li(isect, uLight, &wi, &lightPdf, &visibility);
    if (lightPdf > 0.f && !IsBlack(Li)){
        Color3f f(0.f);
        if (isect.isSurfaceIntersection()){
            f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.normalGeometric);
            scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);
        }
        else{
            const MediumIntersection &mi = (const MediumIntersection &)isect;
            Float p = mi.phase->p(mi.wo, wi);
            f = Color3f(p);
            scatteringPdf = p;
        }
        if (!IsBlack(f)){
            // compute effect of visibility for light source sample
            if (handleMedia){
                Li *= visibility.Tr(scene, sampler);
            } else {
                if (!visibility.Unoccluded(scene)){
                    Li = Color3f(0.f);
                }
            }

            // Add lights's contributions to reflected radiance
            if (!IsBlack(Li)){
                // FIX: no delta light here
                Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                Ld += f * Li * weight / lightPdf;
            }
        }
    }

    // sample BSDF with MIS
    // FIX : no delta light here
    Color3f f(0.f);
    bool sampledSpecular = false;
    if (isect.isSurfaceIntersection()) {
        BxDFType sampledType;
        // FIX: what is this surfaceIntersection here?
        f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf, bsdfFlags, &sampledType);
        f *= AbsDot(wi, isect.normalGeometric);
        sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
    } else {
        // sampled scattered direction for medium interactions
        const MediumIntersection &mi = (const MediumIntersection &)isect;
        Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
        f = Color3f(p);
        scatteringPdf = p;
    }
    if (!IsBlack(f) && scatteringPdf > 0) {
        Float weight = 1;
        if (!sampledSpecular) {
            lightPdf = light.Pdf_Li(isect, wi);
            if (lightPdf == 0.f) return Ld;
            weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
        }

        // Find intersection and compute transmittance
        Intersection lightIsect;
        Ray ray = isect.SpawnRay(wi);
        Color3f Tr(1.f);
        bool foundIntersection = handleMedia ? scene.IntersectTr(ray, sampler, &lightIsect, &Tr)
                                             : scene.Intersect(ray, &lightIsect);

        // add light contribution from material sampling
        Color3f Li(0.f);
        if (foundIntersection) {
            if (lightIsect.objectHit->GetAreaLight() == &light)
                Li = lightIsect.Le(-wi);
        } else
            Li = light.Le(ray);
        if (!IsBlack(Li)) Ld += f * Li * Tr * weight / scatteringPdf;
    }
    return Ld;
}
