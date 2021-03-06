#include "squareplane.h"
#include <warpfunctions.h>

float SquarePlane::Area() const
{
    float area = 1.f * transform.getScale()[0] * transform.getScale()[1];
    return area;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    *tan = glm::normalize(transform.T3() * glm::vec3(1, 0, 0));
    *bit = glm::normalize(transform.T3() * glm::vec3(0, 1, 0));
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}

Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const
{
    glm::vec4 localPoint = glm::vec4(xi[0]-0.5f, xi[1]-0.5f, 0.f, 1.f);
    Point3f worldPoint = glm::vec3(transform.T() *localPoint);
    Normal3f worldNormal = transform.invTransT() * glm::vec3(0.f, 0.f, 1.f);
    Intersection isect;
    isect.point = worldPoint;
    isect.normalGeometric = worldNormal;
    *pdf = 1.f / Area();
    return isect;
}

void SquarePlane::Sample_Photon(Photon &inPhoton, std::shared_ptr<Sampler> sampler, float &pdf) const
{
    // TODO: is direction transformation correct? do we need normalize?
    Point2f xi = sampler->Get2D();
    Point2f xd = sampler->Get2D();
    Vector3f wi = WarpFunctions::squareToHemisphereCosine(xd);
    pdf = WarpFunctions::squareToHemisphereCosinePDF(wi);
    glm::vec4 localPoint = glm::vec4(xi[0]-0.5f, xi[1]-0.5f, 0.f, 1.f);
    Point3f worldPoint = glm::vec3(transform.T() * localPoint);
    Vector3f worldDir = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.f)));
    inPhoton.pos = worldPoint;
    inPhoton.wi = worldDir;
}
