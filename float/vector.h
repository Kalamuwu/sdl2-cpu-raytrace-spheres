#ifndef VECTORH
#define VECTORH

#include <math.h>
#include <stdlib.h>
#include <iostream>

class vec3
{
public:
    vec3() {}
    vec3(float e0, float e1, float e2) { e[0] = e0; e[1] = e1; e[2] = e2; }

    inline float x() const { return e[0]; }
    inline float y() const { return e[1]; }
    inline float z() const { return e[2]; }
    inline float r() const { return e[0]; }
    inline float g() const { return e[1]; }
    inline float b() const { return e[2]; }

    inline bool operator==(const vec3& v2) const;
    inline bool operator!=(const vec3& v2) const;
    inline const vec3& operator+() const { return *this; }
    inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    inline float operator[](int i) const { return e[i]; }
    inline float& operator[](int i) { return e[i]; }

    inline vec3& operator+=(const vec3& v2);
    inline vec3& operator-=(const vec3& v2);
    inline vec3& operator*=(const vec3& v2);
    inline vec3& operator/=(const vec3& v2);
    inline vec3& operator*=(const float t);
    inline vec3& operator/=(const float t);

    inline float squared_length() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }
    inline float length() const {
        return sqrt( squared_length() );
    }

    inline vec3 clamp(const float min, const float max);
    inline vec3 clamp(const vec3& min, const vec3& max);

    alignas(16) float e[4];
};


// std stream operations


inline std::istream& operator>>(std::istream &is, vec3 &t)
{
    is >> t.e[0] >> t.e[1] >> t.e[2];
    return is;
}

inline std::ostream& operator<<(std::ostream &os, const vec3& t)
{
    os << t.e[0] << " " << t.e[1] << " " << t.e[2];
    return os;
}


// arithmetic operations


#ifndef VEC3_EQUALS_EPSILON
#define VEC3_EQUALS_EPSILON 1.0e-9
#endif

inline bool vec3::operator==(const vec3& v2) const
{
    vec3 diff = vec3(
        abs(e[0] - v2.e[0]),
        abs(e[1] - v2.e[1]),
        abs(e[2] - v2.e[2]));
    return
        diff[0] < VEC3_EQUALS_EPSILON &&
        diff[1] < VEC3_EQUALS_EPSILON &&
        diff[2] < VEC3_EQUALS_EPSILON;
}

inline vec3 operator+(const vec3& v1, const vec3& v2)
{
    return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

inline vec3 operator-(const vec3& v1, const vec3& v2)
{
    return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

inline vec3 operator*(const vec3& v1, const vec3& v2)
{
    return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}

inline vec3 operator/(const vec3& v1, const vec3& v2)
{
    return vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

inline vec3 operator*(float t, const vec3& v)
{
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, float t)
{
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator/(vec3 v, float t)
{
    float k = 1.0f / t;
    return vec3(
        v.e[0] * k,
        v.e[1] * k,
        v.e[2] * k);
}

inline vec3& vec3::operator+=(const vec3& v)
{
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
}

inline vec3& vec3::operator-=(const vec3& v)
{
    e[0] -= v.e[0];
    e[1] -= v.e[1];
    e[2] -= v.e[2];
    return *this;
}

inline vec3& vec3::operator*=(const vec3& v)
{
    e[0] *= v.e[0];
    e[1] *= v.e[1];
    e[2] *= v.e[2];
    return *this;
}

inline vec3& vec3::operator/=(const vec3& v)
{
    e[0] /= v.e[0];
    e[1] /= v.e[1];
    e[2] /= v.e[2];
    return *this;
}

inline vec3& vec3::operator*=(const float t)
{
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
}

inline vec3& vec3::operator/=(const float t)
{
    float k = 1.0f/t;
    e[0] *= k;
    e[1] *= k;
    e[2] *= k;
    return *this;
}


// vector operations


inline vec3 vec3::clamp(const float min, const float max)
{
    const float a = (e[0] < min)? min : e[0];
    const float b = (e[1] < min)? min : e[1];
    const float c = (e[2] < min)? min : e[2];
    return vec3(
        (a > max? max : a),
        (b > max? max : b),
        (c > max? max : c)
    );
}

inline vec3 vec3::clamp(const vec3& min, const vec3& max)
{
    const float a = (e[0] < min.e[0])? min.e[0] : e[0];
    const float b = (e[1] < min.e[1])? min.e[1] : e[1];
    const float c = (e[2] < min.e[2])? min.e[2] : e[2];
    return vec3(
        (a > max.e[0]? max.e[0] : a),
        (b > max.e[1]? max.e[1] : b),
        (c > max.e[2]? max.e[2] : c)
    );
}

inline float dot(const vec3& v1, const vec3& v2)
{
    return v1.e[0]*v2.e[0] + v1.e[1]*v2.e[1] + v1.e[2]*v2.e[2];
}

inline vec3 cross(const vec3& v1, const vec3& v2)
{
    return vec3(
           (v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1]),
          -(v1.e[0]*v2.e[2] - v1.e[2]*v2.e[0]),
           (v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0])
    );
}

inline vec3 normalize(vec3 v) { return v / v.length(); }

inline vec3 random_in_unit_sphere()
{
    vec3 p;
    do {
        // pick random point in unit cube
        p = 2.0f*vec3(drand48(),drand48(),drand48()) - vec3(1,1,1);
    // reject while not in unit sphere
    } while (p.squared_length() >= 1);
    return p;
}

inline vec3 reflect(const vec3& v, const vec3& n) { return v - 2.0f*dot(v,n)*n; }

#endif
