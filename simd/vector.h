#ifndef VECTORH
#define VECTORH

#include <immintrin.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#ifndef VEC3_EQUALS_EPSILON
#define VEC3_EQUALS_EPSILON 1.0e-9
#endif

class vec3
{
public:
    vec3() { xmm = _mm_set1_ps(0.0f); }
    vec3(float v0, float v1, float v2) { v[0] = v0; v[1] = v1; v[2] = v2; }
    vec3(__m128 xmm) { this->xmm = xmm; }

    inline float x() const { return v[0]; }
    inline float y() const { return v[1]; }
    inline float z() const { return v[2]; }
    inline float r() const { return v[0]; }
    inline float g() const { return v[1]; }
    inline float b() const { return v[2]; }

    inline bool operator==(const vec3& v2) const;
    inline const vec3& operator+() const { return *this; }
    inline __m128 operator-() const {
        return _mm_xor_ps(_mm_set1_ps(-0.0), xmm);
    }
    inline float operator[](int i) const { return v[i]; }
    inline float& operator[](int i) { return v[i]; }

    inline vec3& operator+=(const vec3& v2);
    inline vec3& operator-=(const vec3& v2);
    inline vec3& operator*=(const vec3& v2);
    inline vec3& operator/=(const vec3& v2);
    inline vec3& operator*=(const float t);
    inline vec3& operator/=(const float t);

    inline float manhattan() const;
    inline float squared_length() const;
    inline float length() const;

    inline vec3 clamp(const float min, const float max) const;
    inline vec3 clamp(const vec3& min, const vec3& max) const;

    union {
        __m128 xmm;
        alignas(16) float v[4];
    };
};


// std stream operations


// inline std::istream& operator>>(std::istream &is, vec3 &t)
// {
//     is >> t.e[0] >> t.e[1] >> t.e[2];
//     return is;
// }
//
// inline std::ostream& operator<<(std::ostream &os, const vec3& t)
// {
//     os << t.e[0] << " " << t.e[1] << " " << t.e[2];
//     return os;
// }


// arithmetic operations


inline bool vec3::operator==(const vec3& v2) const
{
    __m128 diff = _mm_sub_ps(xmm, v2.xmm);
    diff = _mm_andnot_ps(diff, _mm_set1_ps(-0.0));  // abs
    __m128i eq = (__m128i)_mm_cmple_ps(diff, _mm_set1_ps(VEC3_EQUALS_EPSILON));
    uint16_t is_le_epsilon = _mm_movemask_epi8(eq);
    return is_le_epsilon == 0;
}

inline vec3 operator+(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_add_ps(v1.xmm, v2.xmm));
}

inline vec3 operator-(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_sub_ps(v1.xmm, v2.xmm));
}

inline vec3 operator*(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_mul_ps(v1.xmm, v2.xmm));
}

inline vec3 operator/(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_div_ps(v1.xmm, v2.xmm));
}

inline vec3 operator*(float t, const vec3& v)
{
    const __m128 scalar = _mm_set1_ps(t);
    return vec3(_mm_mul_ps(scalar, v.xmm));
}

inline vec3 operator*(const vec3& v, float t)
{
    const __m128 scalar = _mm_set1_ps(t);
    return vec3(_mm_mul_ps(scalar, v.xmm));
}

inline vec3 operator/(vec3 v, float t)
{
    const __m128 scalar = _mm_set_ps1(t);
    return vec3(_mm_div_ps(v.xmm, scalar));
}

inline vec3& vec3::operator+=(const vec3& v)
{
    xmm = _mm_add_ps(xmm, v.xmm);
    return *this;
}

inline vec3& vec3::operator-=(const vec3& v)
{
    xmm = _mm_sub_ps(xmm, v.xmm);
    return *this;
}

inline vec3& vec3::operator*=(const vec3& v)
{
    xmm = _mm_mul_ps(xmm, v.xmm);
    return *this;
}

inline vec3& vec3::operator/=(const vec3& v)
{
    xmm = _mm_div_ps(xmm, v.xmm);
    return *this;
}

inline vec3& vec3::operator*=(const float t)
{
    const __m128 scalar = _mm_set1_ps(t);
    xmm = _mm_mul_ps(xmm, scalar);
    return *this;
}

inline vec3& vec3::operator/=(const float t)
{
    const __m128 scalar = _mm_set1_ps( 1.0f/t );
    xmm = _mm_mul_ps(xmm, scalar);
    return *this;
}


// vector operations

inline float vec3::manhattan() const
{
    return v[0] + v[1] + v[2];
}

inline float vec3::squared_length() const
{
    // note: could be
    // return dot(*this, *this);
    const vec3 squared = vec3(_mm_mul_ps(xmm, xmm));
    return squared.manhattan();
}

inline float vec3::length() const
{
    return sqrt( squared_length() );
}

inline vec3 vec3::clamp(const float min, const float max) const
{
    return vec3(
        _mm_min_ps(
            _mm_max_ps(
                xmm,
                _mm_set1_ps(min)),
            _mm_set1_ps(max)
        ));
}

inline vec3 vec3::clamp(const vec3& min, const vec3& max) const
{
    return vec3(
        _mm_min_ps(
            _mm_max_ps(
                xmm,
                min.xmm),
            max.xmm
        ));
}

inline float dot(const vec3& v1, const vec3& v2)
{
    const vec3 muls = vec3(_mm_mul_ps(v1.xmm, v2.xmm));
    return muls.manhattan();
}

// adapted from Ian Mallet over at geometrian:  https://geometrian.com/programming/tutorials/cross-product/index.php
inline vec3 cross(const vec3& v1, const vec3& v2)
{
    const __m128 tmp0 = _mm_shuffle_ps(v1.xmm, v1.xmm, _MM_SHUFFLE(3,0,2,1));
          __m128 tmp1 = _mm_shuffle_ps(v2.xmm, v2.xmm, _MM_SHUFFLE(3,1,0,2));
          __m128 tmp2 = _mm_mul_ps(tmp0, v1.xmm);
                 tmp1 = _mm_mul_ps(tmp0, tmp1);
                 tmp2 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3,0,2,1));
    return vec3(_mm_sub_ps(tmp1,tmp2));
}

inline vec3 normalize(vec3 v)
{
    const vec3 mul = vec3(_mm_mul_ps(v.xmm, v.xmm));
    __m128 s = _mm_set1_ps(mul.manhattan());
    s = _mm_rsqrt_ps(s);
    return vec3(_mm_mul_ps(v.xmm, s));
}

inline vec3 random_in_unit_sphere()
{
    float x,y,z,sqsum;
    do {
        // pick random point in unit cube -- 2x-1 ; scales [0,1] to [-1,1]
        x = 2.0f * drand48() - 1.0f;
        y = 2.0f * drand48() - 1.0f;
        z = 2.0f * drand48() - 1.0f;
        sqsum = x*x; sqsum += y*y; sqsum += z*z;
    // reject while not in unit sphere; this happens when : sqrt(x^2 + y^2 + z^2) > 1
    } while (sqsum > 1);
    return vec3(x,y,z);
}

inline vec3 reflect(const vec3& v, const vec3& n)
{
    // return v - 2n * dot(v,n)
    const float d = dot(v,n) * 2.0;
    const __m128 scalar = _mm_set1_ps(d);
    const __m128 scaled_n = _mm_mul_ps(n.xmm, scalar);
    return vec3( _mm_sub_ps(v.xmm, scaled_n) );
}

#endif
