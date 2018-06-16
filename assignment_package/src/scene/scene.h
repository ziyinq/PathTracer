#pragma once
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/lights/light.h>
#include <samplers/sampler.h>

class Primitive;
class Material;
class Light;

class Scene
{
public:
    Scene();
    QList<std::shared_ptr<Primitive>> primitives;
    QList<std::shared_ptr<Material>> materials;
    QList<std::shared_ptr<Light>> lights;
    Camera camera;
    Film film;

    void SetCamera(const Camera &c);

    void CreateTestScene();
    void Clear();

    bool Intersect(const Ray& ray, Intersection* isect) const;
    bool IntersectP(const Ray& ray) const;
    bool IntersectTr(Ray ray, std::shared_ptr<Sampler> sampler, Intersection *isect, Color3f *Tr) const;
};
