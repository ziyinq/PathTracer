#pragma once

#include <globals.h>
#include <scene/materials/bsdf.h>
#include <raytracing/ray.h>
#include <scene/scene.h>
#include <scene/geometry/primitive.h>
#include <QList>
#include "media/medium.h"

class Material;
class BSDF;
class Scene;
class PhaseFunction;
//class MediumInterface;
class MediumIntersection;
class Medium;

class Intersection
{
public:
    Intersection();
    Intersection(const Point3f &p, const Vector3f &wo, Float time,
                const MediumInterface &mediumInterface)
        : point(p), time(time), wo(wo), mediumInterface(mediumInterface) {}
    // Ask _objectHit_ to produce a BSDF
    // based on other data stored in this
    // Intersection, e.g. the surface normal
    // and UV coordinates
    bool ProduceBSDF();

    // Returns the light emitted from this Intersection
    // along _wo_, provided that this Intersection is
    // on the surface of an AreaLight. If not, then
    // zero light is returned.
    Color3f Le(const Vector3f& wo) const;

    // Instantiate a Ray that originates from this Intersection and
    // travels in direction d.
    Ray SpawnRay(const Vector3f &d) const;
    Ray SpawnRayTo(const Point3f &p2) const;
    Ray SpawnRayTo(const Intersection &it) const;
    const Medium *GetMedium(const Vector3f &w) const {
        return glm::dot(w, normalGeometric) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }
    const Medium *GetMedium() const {
        assert(mediumInterface.inside == mediumInterface.outside);
        return mediumInterface.inside;
    }

    bool isSurfaceIntersection() const { return normalGeometric != Normal3f(0.f); }
    bool isMediumIntersection () const { return !isSurfaceIntersection(); }

    Point3f point;          // The place at which the intersection occurred
    Normal3f normalGeometric; // The surface normal at the point of intersection, NO alterations like normal mapping applied
    Point2f uv;             // The UV coordinates computed at the intersection
    float t;                  // The parameterization for the ray (in world space) that generated this intersection.
                              // t is equal to the distance from the point of intersection to the ray's origin if the ray's direction is normalized.
    Primitive const * objectHit;     // The object that the ray intersected, or nullptr if the ray hit nothing.
    std::shared_ptr<BSDF> bsdf;// The Bidirection Scattering Distribution Function found at the intersection.

    Vector3f tangent, bitangent; // World-space vectors that form an orthonormal basis with the surface normal.
    MediumInterface mediumInterface;
    Float time;
    Vector3f wo;
};

class MediumIntersection : public Intersection
{
public:
    // MediumInteraction Public Methods
    MediumIntersection() : phase(nullptr) {}
    MediumIntersection(const Point3f &p, const Vector3f &wo, Float time,
                    const Medium *medium, const PhaseFunction *phase)
      : Intersection(p, wo, time, medium), phase(phase) {}

    bool IsValid() const { return phase != nullptr; }

    // MediumInteraction Public Data
    const PhaseFunction *phase;
};

class VisibilityTester {
  public:
    VisibilityTester() {}
    // VisibilityTester Public Methods
    VisibilityTester(const Intersection &p0, const Intersection &p1)
        : p0(p0), p1(p1) {}
    const Intersection &P0() const { return p0; }
    const Intersection &P1() const { return p1; }
    bool Unoccluded(const Scene &scene) const;
    Color3f Tr(const Scene &scene, std::shared_ptr<Sampler> &sampler) const;

  private:
    Intersection p0, p1;
};

