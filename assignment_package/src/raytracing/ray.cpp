#include <raytracing/ray.h>

Ray::Ray(const Point3f &o, const Vector3f &d, Float tMax, Float time, const Medium *medium):
    origin(o),
    direction(d),
    tMax(tMax),
    time(time),
    medium(medium)
{}

Ray::Ray(const glm::vec4 &o, const glm::vec4 &d, const Medium *medium):
    Ray(Point3f(o), Vector3f(d))
{}

Ray::Ray(const Ray &r):
    Ray(r.origin, r.direction, r.tMax, r.time, r.medium)
{}

Ray Ray::GetTransformedCopy(const Matrix4x4 &T) const
{

    glm::vec4 o = glm::vec4(origin, 1);
    glm::vec4 d = glm::vec4(direction, 0);

    o = T * o;
    d = T * d;

    return Ray(o, d);
}
