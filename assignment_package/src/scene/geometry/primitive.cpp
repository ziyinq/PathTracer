#include "primitive.h"

bool Primitive::Intersect(const Ray &r, Intersection *isect) const
{
    if(!shape->Intersect(r, isect)) return false;
    isect->objectHit = this;
    r.length = glm::length(r.origin - isect->point);
    // We create a BSDF for this intersection in our Integrator classes
    // intersection
    if (mediumInterface.IsMediumTransition())
        isect->mediumInterface = mediumInterface;
    else
        isect->mediumInterface = MediumInterface(r.medium);
    return true;
}

bool Primitive::IntersectP(const Ray &r) const
{
    Intersection isect;
    if (shape->Intersect(r, &isect))
    {
        if(isect.t < r.tMax) return true;
    }
    return false;
}

bool Primitive::ProduceBSDF(Intersection *isect) const
{
    if(material)
    {
        material->ProduceBSDF(isect);
        return true;
    }
    return false;
}

const AreaLight* Primitive::GetAreaLight() const
{
    return areaLight.get();
}


const Material* Primitive::GetMaterial() const
{
    return material.get();
}
