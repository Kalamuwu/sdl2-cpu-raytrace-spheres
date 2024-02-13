#ifndef MATERIALH
#define MATERIALH

#include <bits/stdc++.h>
#include <cmath>
#include "vector.h"
#include "ray.h"
#include "hitable.h"

class Material
{
public:
    virtual bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const = 0;
};


// Diffuse -- Lambertian diffuse material. Simulates a rough, matte material.
class Diffuse : public Material
{
public:
    Diffuse(const vec3& a) : albedo(a) {}

    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        vec3 target = pRec.p + pRec.normal + random_in_unit_sphere();
        pRayIn = ray(pRec.p, target-pRec.p);
        pAttenuation = albedo;
        isLightSource = false;
        return true;
    }

    vec3 albedo;
};

// Metal -- Reflects incoming rays mirrored across the normal vector. Simulates a mirror finish.
class Metal : public Material
{
public:
    Metal(const vec3& a, const float f) : albedo(a) { if (f<1) fuzz = f; else fuzz = 1; }

    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        vec3 reflected = reflect(unit_vector(pRayIn.direction()), pRec.normal);
        pRayIn = ray(pRec.p, reflected + fuzz*random_in_unit_sphere());
        pAttenuation = albedo;
        isLightSource = false;
        return (dot(pRayIn.direction(), pRec.normal) > 0);
    }

    vec3 albedo;
    float fuzz;
};


// Emmissive-- Emits more light than it receives
class Emmissive : public Material
{
public:
    Emmissive(const vec3& a, const float s, const bool c) : albedo(a), strength(s), continueTracing(c) {}

    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        vec3 target = pRec.p + pRec.normal + random_in_unit_sphere();
        pRayIn = ray(pRec.p, target-pRec.p);
        pAttenuation = albedo * strength;
        isLightSource = true;
        return continueTracing;
    }

    vec3 albedo;
    float strength;
    bool continueTracing;
};


// Glass - Calculates internal reflections and refractions
class Glass : public Material
{
public:
    Glass(const vec3& a, const float ri) : albedo(a), ref_idx(ri) {}

    // This was stolen from Peter Shirley's Ray Tracing in One Weekend. Don't
    // ask me how it works. I have a basic understanding but not enough to
    // teach anyone else.
    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        vec3 outward_normal;
        vec3 reflected = reflect(pRayIn.direction(), pRec.normal);
        float ni_over_nt;
        pAttenuation = albedo;
        vec3 refracted;
        float reflect_prob;
        float cosine;
        if (dot(pRayIn.direction(), pRec.normal) > 0)
        {
            outward_normal = -pRec.normal;
            ni_over_nt = ref_idx;
            cosine = ref_idx * dot(pRayIn.direction(), pRec.normal) / pRayIn.direction().length();
        }
        else
        {
            outward_normal = pRec.normal;
            ni_over_nt = 1.0f / ref_idx;
            cosine = -dot(pRayIn.direction(), pRec.normal) / pRayIn.direction().length();
        }
        if (this->refract(pRayIn.direction(), outward_normal, ni_over_nt, refracted))
        {
            reflect_prob = this->schlick(cosine, ref_idx);
        }
        else reflect_prob = 1.0f;
        pRayIn = ray(pRec.p, (drand48() < reflect_prob)? reflected : refracted);
        isLightSource = false;
        return true;
    }

    // Glass has reflectivity that varies with viewing angle. this is a massive
    // ugly equation, but luckily Christophe Schlick came up with this simple
    // polynomial approximation.
    float schlick(float cosine, float ref_idx) const
    {
        float r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine), 5);
    }

    // determines whether the viewing angle is steep enough for total internal
    // reflection (zero refraction) and refracts. this math is loosely based on
    // snells law:   n sin(theta) = n' sin(theta')
    // common values:  air=1, glass=1.3-1.7, diamond=2.4
    bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) const
    {
        vec3 uv = unit_vector(v);
        float dt = dot(uv, n);
        float discriminant = 1.0f - ni_over_nt*ni_over_nt*(1-dt*dt);
        if (discriminant > 0) {
            refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
            return true;
        }
        else return false;
    }

    vec3 albedo;
    float ref_idx;

};


// Translucent -- Somewhat transparent, tints incoming light
class Translucent : public Material
{
public:
    Translucent(const vec3& a, const float t, const float s) : albedo(a) {
        translucency = (t>1) + (t>=0 && t<=1)*t;
        scattering = (scattering>1) + (scattering<=1 && scattering>=0)*scattering;
    }

    // PpRecise method, which guarantees v = v1 when t = 1. This method is monotonic only when v0 * v1 < 0.
    // Lerping between same values might not produce the same value
    static float lerp(float v0, float v1, float t) {
        return (1 - t) * v0 + t * v1;
    }

    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        float cosine = (dot(pRayIn.direction(), pRec.normal)) / (pRayIn.direction().length());
        cosine = 0.5f*(cosine+1);
        pRayIn = ray(pRec.p, pRec.p + pRayIn.direction() + scattering*5*random_in_unit_sphere());
        pAttenuation = albedo;
        pAttenuation[0] = lerp(0.0, pAttenuation[0], cosine) + translucency;
        pAttenuation[1] = lerp(0.0, pAttenuation[1], cosine) + translucency;
        pAttenuation[2] = lerp(0.0, pAttenuation[2], cosine) + translucency;
        isLightSource = false;
        return true;
    }

    vec3 albedo;
    float translucency;
    float scattering;
};


// Normals -- Visualizes normals
class Normals : public Material
{
public:
    Normals() {}

    bool scatter(ray &pRayIn, const hit_record &pRec, vec3 &pAttenuation, bool &isLightSource) const
    {
        pAttenuation = pRec.normal;
        isLightSource = true;
        return false;
    }
};

#endif
