#pragma once
#include <globals.h>
//#include "media/medium.h"

class Medium;

class Ray
{
public:
    Ray() : tMax(INFINITY), time(0.f), medium(nullptr) {}
    Ray(const Point3f &o, const Vector3f &d, Float tMax = INFINITY, Float time = 0.f, const Medium *medium = nullptr);
    Ray(const glm::vec4 &o, const glm::vec4 &d, const Medium *medium = nullptr);
    Ray(const Ray &r);

    Point3f operator()(Float t) const { return origin + direction*t; }
    //Return a copy of this ray that has been transformed
    //by the input transformation matrix.
    Ray GetTransformedCopy(const Matrix4x4 &T) const;

    Point3f origin;
    Vector3f direction;
    mutable Float length;
    const Medium *medium;
    mutable Float tMax;
    Float time;
};
