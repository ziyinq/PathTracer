#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

glm::vec3 WarpFunctions::squareToDiskUniform(const glm::vec2 &sample)
{
    float radius = sqrt(sample[0]);
    float angle = 2.f * Pi * sample[1];
    float new_x = radius * cos(angle);
    float new_y = radius * sin(angle);
    glm::vec3 result(new_x, new_y, 0.f);
    return result;
}

glm::vec3 WarpFunctions::squareToDiskConcentric(const glm::vec2 &sample)
{
    float phi, r, u, v;
    float a = 2.f * sample[0] - 1.f;
    float b = 2.f * sample[1] - 1.f;
    if (a > -b)
    {
        if (a > b)
        {
            r = a;
            phi = (Pi / 4.f) * (b/a);
        }
        else
        {
            r = b;
            phi = (Pi / 4.f) * (2.f - (a/b));
        }
    }
    else
    {
        if (a < b)
        {
            r = -a;
            phi = (Pi / 4.f) * (4.f + (b/a));
        }
        else
        {
            r = -b;
            if (b != 0)
                phi = (Pi / 4.f) * (6.f - (a/b));
            else
                phi = 0;
        }
    }
    u = r * cos(phi);
    v = r * sin(phi);
    glm::vec3 result(u, v, 0.f);
    return result;
}

float WarpFunctions::squareToDiskPDF(const glm::vec3 &sample)
{
    return InvPi;
}

glm::vec3 WarpFunctions::squareToSphereUniform(const glm::vec2 &sample)
{
    float z = 1 - 2*sample[0];
    float x = cos(2*Pi*sample[1])*sqrt(1-z*z);
    float y = sin(2*Pi*sample[1])*sqrt(1-z*z);
    return glm::vec3(x,y,z);
}

float WarpFunctions::squareToSphereUniformPDF(const glm::vec3 &sample)
{
    return Inv4Pi;
}

glm::vec3 WarpFunctions::squareToSphereCapUniform(const glm::vec2 &sample, float thetaMin)
{
    float z = 1.f - 2.f * sample[0] * (180.f - thetaMin)/180.f;
    float x = cos(2*Pi*sample[1])*sqrt(1-z*z);
    float y = sin(2*Pi*sample[1])*sqrt(1-z*z);
    return glm::vec3(x,y,z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const glm::vec3 &sample, float thetaMin)
{
    float theta = Pi*(180-thetaMin)/180;
    return 1.f/(2.f*Pi*(1.f-cos(theta)));
}

glm::vec3 WarpFunctions::squareToHemisphereUniform(const glm::vec2 &sample)
{
    float z = sample[0];
    float x = cos(2*Pi*sample[1])*sqrt(1-z*z);
    float y = sin(2*Pi*sample[1])*sqrt(1-z*z);
    return glm::vec3(x,y,z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const glm::vec3 &sample)
{
    return Inv2Pi;
}

glm::vec3 WarpFunctions::squareToHemisphereCosine(const glm::vec2 &sample)
{
    glm::vec2 disk = glm::vec2(squareToDiskConcentric(sample));
    float x = disk[0];
    float y = disk[1];
    float z = std::max(0.f, sqrtf(1.f - x*x - y*y));
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const glm::vec3 &sample)
{
    float costheta = sample[2];
    return costheta*InvPi;
}
