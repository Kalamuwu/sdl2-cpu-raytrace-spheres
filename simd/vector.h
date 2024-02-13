#ifndef VECTORH
#define VECTORH

#include <immintrin.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#define vecsum(a) _mm_extract_ps(a, 0) + _mm_extract_ps(a, 1) + _mm_extract_ps(a, 2)

class vec3
{
public:
    vec3() { xmm_ = _mm_set1_ps(0.0f); }
    vec3(float e0, float e1, float e2) { xmm_ = _mm_set_ps(e0, e1, e2, 0); }
    vec3(__m128 ex) { xmm_ = ex; }

    inline float x() const { return _mm_extract_ps(xmm_, 0); }
    inline float y() const { return _mm_extract_ps(xmm_, 1); }
    inline float z() const { return _mm_extract_ps(xmm_, 2); }
    inline float r() const { return _mm_extract_ps(xmm_, 0); }
    inline float g() const { return _mm_extract_ps(xmm_, 1); }
    inline float b() const { return _mm_extract_ps(xmm_, 2); }

    inline bool operator==(const vec3& v2) const;
    inline const vec3& operator+() const { return *this; }
    inline __m128 operator-() const {
        __m128 negflag = _mm_set1_ps(-0.0f);
        return _mm_xor_ps(negflag, xmm_);
    }
    //inline float operator[](int i) const { return e[i]; }
    //inline float& operator[](int i) { return e[i]; }

    inline vec3& operator+=(const vec3& v2);
    inline vec3& operator-=(const vec3& v2);
    inline vec3& operator*=(const vec3& v2);
    inline vec3& operator/=(const vec3& v2);
    inline vec3& operator*=(const float t);
    inline vec3& operator/=(const float t);

    inline float squared_length() const;
    inline float length() const;

    inline vec3 clamp(const float min, const float max) const;
    inline vec3 clamp(const vec3& min, const vec3& max) const;

    __m128 xmm_;
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


#ifndef VEC3_EQUALS_EPSILON
#define VEC3_EQUALS_EPSILON 1.0e-9
#endif

inline bool vec3::operator==(const vec3& v2) const
{
    __m128 epsilon = _mm_set1_ps(VEC3_EQUALS_EPSILON);
    __m128 diff = _mm_sub_ps(xmm_, v2.xmm_);
    __m128 diff_abs = _mm_andnot_ps(_mm_set1_ps(-0.0), diff);
    __m128i eq = (__m128i)_mm_cmple_ps(diff_abs, epsilon);
    uint16_t is_le_epsilon = _mm_movemask_epi8(eq);
    return is_le_epsilon == 0;
}

inline vec3 operator+(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_add_ps(v1.xmm_, v2.xmm_));
}

inline vec3 operator-(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_sub_ps(v1.xmm_, v2.xmm_));
}

inline vec3 operator*(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_mul_ps(v1.xmm_, v2.xmm_));
}

inline vec3 operator/(const vec3& v1, const vec3& v2)
{
    return vec3(_mm_div_ps(v1.xmm_, v2.xmm_));
}

inline vec3 operator*(float t, const vec3& v)
{
    const __m128 scalar = _mm_set1_ps(t);
    return vec3(_mm_mul_ps(scalar, v.xmm_));
}

inline vec3 operator*(const vec3& v, float t)
{
    const __m128 scalar = _mm_set1_ps(t);
    return vec3(_mm_mul_ps(scalar, v.xmm_));
}

inline vec3 operator/(vec3 v, float t)
{
    const __m128 scalar = _mm_set_ps1(t);
    return vec3(_mm_div_ps(v.xmm_, scalar));
}

inline vec3& vec3::operator+=(const vec3& v)
{
    xmm_ = _mm_add_ps(xmm_, v.xmm_);
    return *this;
}

inline vec3& vec3::operator-=(const vec3& v)
{
    xmm_ = _mm_sub_ps(xmm_, v.xmm_);
    return *this;
}

inline vec3& vec3::operator*=(const vec3& v)
{
    xmm_ = _mm_mul_ps(xmm_, v.xmm_);
    return *this;
}

inline vec3& vec3::operator/=(const vec3& v)
{
    xmm_ = _mm_div_ps(xmm_, v.xmm_);
    return *this;
}

inline vec3& vec3::operator*=(const float t)
{
    const __m128 scalar = _mm_set1_ps(t);
    xmm_ = _mm_mul_ps(xmm_, scalar);
    return *this;
}

inline vec3& vec3::operator/=(const float t)
{
    const __m128 scalar = _mm_set1_ps( 1.0f/t );
    xmm_ = _mm_mul_ps(xmm_, scalar);
    return *this;
}


// vector operations


inline float vec3::squared_length() const {
    const __m128 squares = _mm_mul_ps(xmm_, xmm_);
    return vecsum(squares);
}

inline float vec3::length() const {
    return sqrt( squared_length() );
}

inline vec3 vec3::clamp(const float min, const float max) const
{
    const __m128 min_xmm = _mm_set_ps1(min);
    const __m128 max_xmm = _mm_set_ps1(max);
    const __m128 mins = _mm_max_ps(xmm_, min_xmm);
    const __m128 maxs = _mm_min_ps(mins, max_xmm);
    return vec3(maxs);
}

inline vec3 vec3::clamp(const vec3& min, const vec3& max) const
{
    const __m128 mins = _mm_max_ps(xmm_, min.xmm_);
    const __m128 maxs = _mm_min_ps(mins, max.xmm_);
    return vec3(maxs);
}

inline float dot(const vec3& v1, const vec3& v2)
{
    __m128 muls = _mm_mul_ps(v1.xmm_, v2.xmm_);
    return vecsum(muls);
}

// stolen from Ian Mallet over at geometrian:  https://geometrian.com/programming/tutorials/cross-product/index.php
inline vec3 cross(const vec3& v1, const vec3& v2)
{
    __m128 tmp0 = _mm_shuffle_ps(v1.xmm_, v1.xmm_, _MM_SHUFFLE(3,0,2,1));
    __m128 tmp1 = _mm_shuffle_ps(v2.xmm_, v2.xmm_, _MM_SHUFFLE(3,1,0,2));
    __m128 tmp2 = _mm_mul_ps(tmp0, v1.xmm_);
    __m128 tmp3 = _mm_mul_ps(tmp0, v2.xmm_);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3,0,2,1));
    return vec3(_mm_sub_ps(tmp3,tmp4));
}

inline vec3 normalize(vec3 v) {
    float k = 1.0f / v.length();
    return v * k;
}

inline vec3 random_in_unit_sphere()
{
    const __m128 m = _mm_set1_ps(2.0f);
    const __m128 a = _mm_set1_ps(-1.0f);
    __m128 p, r, psq;
    do {
        // pick random point in unit cube
        r = _mm_set_ps(drand48(), drand48(), drand48(), 0);
        p = _mm_fmadd_ps(r, m, a);  // 2x-1 ; scales [0,1] to [-1,1]
        psq = _mm_mul_ps(p, p);
    // reject while not in unit sphere; this happens when : sqrt(x^2 + y^2 + z^2) > 1
    } while (vecsum(psq) > 1);
    return p;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    // return v - 2n * dot(v,n)
    const float d = dot(v,n) * 2.0;
    const __m128 scalar = _mm_set1_ps(d);
    const __m128 scaled_n = _mm_mul_ps(n.xmm_, scalar);
    return vec3( _mm_sub_ps(v.xmm_, scaled_n) );
}

#endif
