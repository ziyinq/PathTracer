#include "shape.h"
#include <QDateTime>
#include <warpfunctions.h>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    Intersection isect = Sample(xi, pdf);
    Vector3f wi = glm::normalize(ref.point - isect.point);
    float cosineTheta = AbsDot(glm::normalize(isect.normalGeometric), wi);

    if(cosineTheta == 0.f)
    {
        *pdf = 0.f;
    }
    // Convert light sample weight to solid angle measure
    *pdf *= glm::length2(ref.point - isect.point)
            / (cosineTheta);
    return isect;
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi) const
{
    Ray ray = ref.SpawnRay(wi);
    Intersection isect;
    // Ignore any alpha textures used for trimming the shape when performing
    // this intersection. Hack for the "San Miguel" scene, where this is used
    // to make an invisible area light.
    if (!Intersect(ray, &isect)) return 0.f;

    float cosineTheta = AbsDot(glm::normalize(isect.normalGeometric), -wi);
    if(cosineTheta == 0.f)
    {
        return 0.f;
    }
    // Convert light sample weight to solid angle measure
    float pdf = glm::length2(ref.point-isect.point) /
                (cosineTheta * Area());
    return pdf;
}

bool Shape::IntersectP(const Ray &ray, bool testAlphaTexture) const
{
    Intersection isect;
    return Intersect(ray, &isect);
}
