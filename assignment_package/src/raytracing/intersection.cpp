#include <raytracing/intersection.h>

Intersection::Intersection():
    point(Point3f(0)),
    normalGeometric(Normal3f(0)),
    uv(Point2f(0)),
    t(-1),
    objectHit(nullptr),
    bsdf(nullptr),
    tangent(0.f), bitangent(0.f)
{}

bool Intersection::ProduceBSDF()
{
    return objectHit->ProduceBSDF(this);
}

Color3f Intersection::Le(const Vector3f &wo) const
{
    const AreaLight* light = objectHit->GetAreaLight();
    return light ? light->L(*this, wo) : Color3f(0.f);
}

Ray Intersection::SpawnRay(const Vector3f &d) const
{
    Vector3f originOffset = normalGeometric * RayEpsilon;
    // Make sure to flip the direction of the offset so it's in
    // the same general direction as the ray direction
    originOffset = (glm::dot(d, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f o(this->point + originOffset);
    return Ray(o, d);
}

Ray Intersection::SpawnRayTo(const Point3f &p2) const
{
    Vector3f thisDirection = p2 - this->point;
    Vector3f originOffset = normalGeometric * RayEpsilon;
    // Make sure to flip the direction of the offset so it's in
    // the same general direction as the ray direction
    originOffset = (glm::dot(thisDirection, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f o(this->point + originOffset);
    Vector3f d = p2 - o;
    Float tMax = glm::length(d);
    return Ray(o, glm::normalize(d), tMax, time, GetMedium(d));
}

Ray Intersection::SpawnRayTo(const Intersection &it) const
{
    Vector3f thisDirection = it.point - this->point;
    Vector3f originOffset = normalGeometric * RayEpsilon;
    // Make sure to flip the direction of the offset so it's in
    // the same general direction as the ray direction
    originOffset = (glm::dot(thisDirection, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f origin(this->point + originOffset);

    Vector3f secondD = origin - it.point;
    Vector3f secondOffsect = it.normalGeometric * RayEpsilon;
    secondOffsect = (glm::dot(secondD, it.normalGeometric) > 0) ? secondOffsect : -secondOffsect;
    Point3f target(it.point + secondOffsect);
    Vector3f d = target - origin;
    Float tMax = glm::length(d);
    return Ray(origin, glm::normalize(d), tMax, time, GetMedium(d));
}

// Scene : intersect P
bool VisibilityTester::Unoccluded(const Scene &scene) const {
    return !scene.IntersectP(p0.SpawnRayTo(p1));
}

// TODO: surfaceIntersection for isect here???
Color3f VisibilityTester::Tr(const Scene &scene, std::shared_ptr<Sampler> &sampler) const {
    Ray ray(p0.SpawnRayTo(p1));
    Color3f Tr(1.f);
    while (true) {
        Intersection isect;
        isect.t = ray.tMax;
        bool hitSurface = scene.Intersect(ray, &isect);
        // Handle opaque surface along ray's path
        if (hitSurface && isect.objectHit->GetMaterial() != nullptr)
            return Color3f(0.0f);

        // Update transmittance for current ray segment
        if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);

        // Generate next ray segment or return final transmittance
        if (!hitSurface) break;
        ray = isect.SpawnRayTo(p1);
    }
    return Tr;
}
