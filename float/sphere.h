#ifndef SPHEREH
#define SPHEREH

#include "vector.h"
#include "ray.h"
#include "hitable.h"
#include "material.h"

class Sphere: public Hitable
{
public:
    Sphere() {};
    Sphere(vec3 cen, float r, Material* mat) : center(cen), radius(r), pMat(mat) {};
    bool hit(const ray & r, float t_min, float t_max, hit_record & rec) const override;

    vec3 center;
    float radius;
    Material* pMat;
};

bool Sphere::hit(const ray &rayIn, float tMin, float tMax, hit_record &rec) const
{
    vec3 oc = rayIn.origin() - center;
    float a = dot(rayIn.direction(), rayIn.direction());
    float b = dot(oc, rayIn.direction());
    float c = dot(oc, oc) - radius*radius;
    float discrim = b*b - a*c;
    if (discrim > 0)
    {
        // try "minus" quadratic root
        float temp = (-b - sqrt(discrim)) / a;
        if (temp < tMax && temp > tMin)
        {
            rec.t = temp;
            rec.p = rayIn.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.pMat = pMat;
            return true;
        }
        // try "plus" quadratic root
        temp = (-b + sqrt(discrim)) / a;
        if (temp < tMax && temp > tMin)
        {
            rec.t = temp;
            rec.p = rayIn.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.pMat = pMat;
            return true;
        }
    }
    return false;
}

#endif
