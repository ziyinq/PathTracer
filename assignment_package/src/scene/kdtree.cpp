#include "kdtree.h"

KDNode::KDNode()
    : leftChild(nullptr), rightChild(nullptr), axis(0), minCorner(), maxCorner(), particles()
{}

KDNode::~KDNode()
{
    delete leftChild;
    delete rightChild;
}

KDTree::KDTree()
    : root(nullptr)
{}

KDTree::~KDTree()
{
    delete root;
}

// Comparator functions you can use with std::sort to sort vec3s along the cardinal axes
bool xSort(Photon a, Photon b) { return a.pos.x < b.pos.x; }
bool ySort(Photon a, Photon b) { return a.pos.y < b.pos.y; }
bool zSort(Photon a, Photon b) { return a.pos.z < b.pos.z; }
bool xTrisSort(Triangle* a, Triangle* b) { return a->middlePos.x < b->middlePos.x; }
bool yTrisSort(Triangle* a, Triangle* b) { return a->middlePos.y < b->middlePos.y; }
bool zTrisSort(Triangle* a, Triangle* b) { return a->middlePos.z < b->middlePos.z; }

void KDTree::build(const std::vector<Photon> *points)
{
    minCorner = getMinCorner(*points);
    maxCorner = getMaxCorner(*points);
    root = buildHelper(*points, 0);
    std::cout << "KDTree Build Finished!" << std::endl;
}

void KDTree::buildTris(const QList<Triangle *> &tris)
{
    minCorner = getTriMinCorner(tris);
    maxCorner = getTriMaxCorner(tris);
    root = buildTrisHelper(tris, 0);
    std::cout << "KDTree Build Finished!" << std::endl;
}

glm::vec3 KDTree::getTriMaxCorner(const QList<Triangle *> &tris)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (auto t : tris)
    {
        xarray.push_back(t->points[0].x);
        xarray.push_back(t->points[1].x);
        xarray.push_back(t->points[2].x);
        yarray.push_back(t->points[0].y);
        yarray.push_back(t->points[1].y);
        yarray.push_back(t->points[2].y);
        zarray.push_back(t->points[0].z);
        zarray.push_back(t->points[1].z);
        zarray.push_back(t->points[2].z);
    }
    auto xresult = std::max_element(xarray.begin(), xarray.end());
    auto yresult = std::max_element(yarray.begin(), yarray.end());
    auto zresult = std::max_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

glm::vec3 KDTree::getTriMinCorner(const QList<Triangle *> &tris)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (auto t : tris)
    {
        xarray.push_back(t->points[0].x);
        xarray.push_back(t->points[1].x);
        xarray.push_back(t->points[2].x);
        yarray.push_back(t->points[0].y);
        yarray.push_back(t->points[1].y);
        yarray.push_back(t->points[2].y);
        zarray.push_back(t->points[0].z);
        zarray.push_back(t->points[1].z);
        zarray.push_back(t->points[2].z);
    }
    auto xresult = std::min_element(xarray.begin(), xarray.end());
    auto yresult = std::min_element(yarray.begin(), yarray.end());
    auto zresult = std::min_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

glm::vec3 KDTree::getMaxCorner(const std::vector<Photon> &points)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (unsigned int i = 0; i < points.size(); i++)
    {
        xarray.push_back(points[i].pos.x);
        yarray.push_back(points[i].pos.y);
        zarray.push_back(points[i].pos.z);
    }
    auto xresult = std::max_element(xarray.begin(), xarray.end());
    auto yresult = std::max_element(yarray.begin(), yarray.end());
    auto zresult = std::max_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

glm::vec3 KDTree::getMinCorner(const std::vector<Photon> &points)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (unsigned int i = 0; i < points.size(); i++)
    {
        xarray.push_back(points[i].pos.x);
        yarray.push_back(points[i].pos.y);
        zarray.push_back(points[i].pos.z);
    }
    auto xresult = std::min_element(xarray.begin(), xarray.end());
    auto yresult = std::min_element(yarray.begin(), yarray.end());
    auto zresult = std::min_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

KDNode* KDTree::buildHelper(const std::vector<Photon> &points, int depth)
{
    KDNode *mynode = new KDNode();
    std::vector<Photon> sortPoints = points;
    unsigned int axis = depth % 3;
    if (sortPoints.size() > 1)
    {
        if (axis == 0)
        {
            std::sort(sortPoints.begin(), sortPoints.end(), xSort);
        }
        else if (axis == 1)
        {
            std::sort(sortPoints.begin(), sortPoints.end(), ySort);
        }
        else
        {
            std::sort(sortPoints.begin(), sortPoints.end(), zSort);
        }
        int median = sortPoints.size() / 2;
        std::vector<Photon> leftPoints(sortPoints.begin(), sortPoints.begin()+median);
        std::vector<Photon> rightPoints(sortPoints.begin()+median, sortPoints.end());
        mynode->axis = axis;
        mynode->minCorner = getMinCorner(sortPoints);
        mynode->maxCorner = getMaxCorner(sortPoints);
        mynode->leftChild = buildHelper(leftPoints, depth+1);
        mynode->rightChild = buildHelper(rightPoints, depth+1);
    }
    else
    {
        mynode->axis = axis;
        mynode->particles = sortPoints;
        mynode->minCorner = getMinCorner(sortPoints);
        mynode->maxCorner = getMaxCorner(sortPoints);
    }
    return mynode;
}

KDNode* KDTree::buildTrisHelper(const QList<Triangle *> &tris, int depth)
{
    KDNode *mynode = new KDNode();
    QList<Triangle*> sortTris = tris;
    unsigned int axis = depth & 3;
    if (sortTris.size() > 1)
    {
        if (axis == 0)
        {
            std::sort(sortTris.begin(), sortTris.end(), xTrisSort);
        }
        else if (axis == 1)
        {
            std::sort(sortTris.begin(), sortTris.end(), yTrisSort);
        }
        else
        {
            std::sort(sortTris.begin(), sortTris.end(), zTrisSort);
        }
        int median = sortTris.size() / 2;
        QList<Triangle*> leftPoints(sortTris.mid(0, median));
        QList<Triangle*> rightPoints(sortTris.mid(median));
        mynode->axis = axis;
        mynode->minCorner = getTriMinCorner(sortTris);
        mynode->maxCorner = getTriMaxCorner(sortTris);
        mynode->leftChild = buildTrisHelper(leftPoints, depth+1);
        mynode->rightChild = buildTrisHelper(rightPoints, depth+1);
    }
    else
    {
        mynode->axis = axis;
        mynode->triangles = sortTris;
        mynode->minCorner = getTriMinCorner(sortTris);
        mynode->maxCorner = getTriMaxCorner(sortTris);
    }
    return mynode;
}

std::vector<Photon> KDTree::particlesInSphere(glm::vec3 c, float r) const
{
    std::vector<Photon> list;
    rangeSearch(root, list, c, r);
    return list;
}

void KDTree::rangeSearch(KDNode *node, std::vector<Photon> &list, glm::vec3 c, float r) const
{
    if (node->particles.size() != 0 && (node->leftChild == nullptr && node->rightChild == nullptr))
    {
        glm::vec3 p = node->particles[0].pos;
        if (glm::distance(p, c) <= r)
        {
            list.push_back(node->particles[0]);
        }
    }
    else
    {
        if (intersect(node->leftChild, c, r))
        {
            rangeSearch(node->leftChild, list, c, r);
        }
        if (intersect(node->rightChild, c, r))
        {
            rangeSearch(node->rightChild, list, c, r);
        }
    }
}

bool KDTree::findIntersectTris(const Ray &r, QList<Triangle *> &tris)
{
    if (hitBoundBox(minCorner, maxCorner, r)){
        traverse(root, r, tris);
        if (tris.size() != 0) return true;
    }
    return false;
}

void KDTree::traverse(KDNode *node, const Ray &r, QList<Triangle*> &tris)
{
    if (node->triangles.size() != 0 && (node->leftChild == nullptr && node->rightChild == nullptr))
    {
        tris.append(node->triangles[0]);
    }
    else
    {
        if (hitBoundBox(node->leftChild->minCorner, node->rightChild->maxCorner, r))
        {
            traverse(node->leftChild, r, tris);
        }
        if (hitBoundBox(node->rightChild->minCorner, node->rightChild->maxCorner, r))
        {
            traverse(node->rightChild, r, tris);
        }
    }
}

bool KDTree::hitBoundBox(const Point3f &minC, const Point3f &maxC, const Ray &r)
{
    Point3f bounds[2];
    bounds[0] = minC;
    bounds[1] = maxC;
    Vector3f invdir = 1.f / r.direction;
    int sign[3];
    sign[0] = (invdir.x < 0);
    sign[1] = (invdir.y < 0);
    sign[2] = (invdir.z < 0);
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (bounds[sign[0]].x - r.origin.x) * invdir.x;
    tmax = (bounds[1-sign[0]].x - r.origin.x) * invdir.x;
    tymin = (bounds[sign[1]].y - r.origin.y) * invdir.y;
    tymax = (bounds[1-sign[1]].y - r.origin.y) * invdir.y;

    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    tzmin = (bounds[sign[2]].z - r.origin.z) * invdir.z;
    tzmax = (bounds[1-sign[2]].z - r.origin.z) * invdir.z;

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    return true;

}

bool KDTree::intersect(KDNode *node, glm::vec3 c, float r) const
{
    float sumDist = 0;
    float r2 = r*r;
    for (int i = 0; i < 3; i++)
    {
        if (c[i] < node->minCorner[i]) sumDist += glm::length2(c[i] - node->minCorner[i]);
        else if (c[i] > node->maxCorner[i]) sumDist += glm::length2(c[i] - node->maxCorner[i]);
    }
    return sumDist <= r2;
}

void KDTree::clear()
{
    delete root;
    root = nullptr;
}
