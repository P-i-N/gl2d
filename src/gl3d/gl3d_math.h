#pragma once

namespace gl3d {

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xvec2
{
  typedef T type;
  T x = 0, y = 0;
  const T *data() const { return &x; }

  xvec2() { }
  xvec2(const xvec2 &copy): x(copy.x), y(copy.y) { }

  template <typename T2>
  xvec2(T2 _x, T2 _y): x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }

  xvec2 &operator=(const xvec2 &rhs) { x = rhs.x; y = rhs.y; return *this; }
  xvec2 operator*(T scale) const { return { x * scale, y * scale }; }

  T length_sq() const { return x*x + y*y; }
  T length() const { return sqrt(length_sq()); }

  static const xvec2 &unit_x() { static xvec2 v(1, 0); return v; }
  static const xvec2 &unit_y() { static xvec2 v(0, 1); return v; }
  static const xvec2 &one()    { static xvec2 v(1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xvec3
{
  typedef T type;
  T x = 0, y = 0, z = 0;
  const T *data() const { return &x; }

  xvec3() { }
  xvec3(const xvec3 &copy): x(copy.x), y(copy.y), z(copy.z) { }

  template <typename T2>
  xvec3(T2 _x, T2 _y, T2 _z): x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) { }

  xvec3 &operator=(const xvec3 &rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
  xvec3 operator*(T scale) const { return { x * scale, y * scale, z * scale }; }

  T length_sq() const { return x*x + y*y + z*z; }
  T length() const { return sqrt(length_sq()); }

  static const xvec3 &unit_x() { static xvec3 v(1, 0, 0); return v; }
  static const xvec3 &unit_y() { static xvec3 v(0, 1, 0); return v; }
  static const xvec3 &unit_z() { static xvec3 v(0, 0, 1); return v; }
  static const xvec3 &one()    { static xvec3 v(1, 1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xvec4
{
  typedef T type;
  T x = 0, y = 0, z = 0, w = 0;
  const T *data() const { return &x; }

  xvec4() { }
  xvec4(const xvec4 &copy): x(copy.x), y(copy.y), z(copy.z), w(copy.w) { }

  template <typename T2>
  xvec4(T2 _x, T2 _y, T2 _z, T2 _w)
    : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)), w(static_cast<T>(_w)) { }

  explicit xvec4(uint32_t argb)
    : x(((argb >> 16) & 0xFFu) / 255.0f), y(((argb >> 8) & 0xFFu) / 255.0f)
    , z((argb & 0xFFu) / 255.0f), w(((argb >> 24) & 0xFFu) / 255.0f) { }

  xvec4 &operator=(const xvec4 &rhs) { x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w; return *this; }
  xvec4 operator*(T scale) const { return { x * scale, y * scale, z * scale, w * scale }; }

  T length_sq() const { return x*x + y*y + z*z + w*w; }
  T length() const { return sqrt(length_sq()); }

  static const xvec4 &unit_x() { static xvec4 v(1, 0, 0, 0); return v; }
  static const xvec4 &unit_y() { static xvec4 v(0, 1, 0, 0); return v; }
  static const xvec4 &unit_z() { static xvec4 v(0, 0, 1, 0); return v; }
  static const xvec4 &unit_w() { static xvec4 v(0, 0, 0, 1); return v; }
  static const xvec4 &one()    { static xvec4 v(1, 1, 1, 1); return v; }
  static const xvec4 &red()    { static xvec4 v(1, 0, 0, 1); return v; }
  static const xvec4 &green()  { static xvec4 v(0, 1, 0, 1); return v; }
  static const xvec4 &blue()   { static xvec4 v(0, 0, 1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xmat4
{
  typedef T type;
  T m[16];
  const T *data() const { return m; }
  T operator[](size_t index) const { return m[index]; }

  xmat4()
  {
    m[0] = m[5] = m[10] = m[15] = static_cast<T>(1);
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0;
  }

  xmat4(const xmat4 &copy) { memcpy(m, copy.m, sizeof(T) * 16); }
  xmat4(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8, T m9, T m10, T m11, T m12, T m13, T m14, T m15)
  {
    m[0] = m0; m[1] = m1; m[2] = m2; m[3] = m3; m[4] = m4; m[5] = m5; m[6] = m6; m[7] = m7;
    m[8] = m8; m[9] = m9; m[10] = m10; m[11] = m11; m[12] = m12; m[13] = m13; m[14] = m14; m[15] = m15;
  }

  xmat4 &operator=(const xmat4 &rhs) { memcpy(m, rhs.m, sizeot(T) * 16); return *this; }

  xmat4 operator*(const xmat4 &n) const
  {
    return xmat4(m[0]*n[0]  + m[4]*n[1]  + m[8]*n[2]  + m[12]*n[3],   m[1]*n[0]  + m[5]*n[1]  + m[9]*n[2]  + m[13]*n[3],   m[2]*n[0]  + m[6]*n[1]  + m[10]*n[2]  + m[14]*n[3],   m[3]*n[0]  + m[7]*n[1]  + m[11]*n[2]  + m[15]*n[3],
                 m[0]*n[4]  + m[4]*n[5]  + m[8]*n[6]  + m[12]*n[7],   m[1]*n[4]  + m[5]*n[5]  + m[9]*n[6]  + m[13]*n[7],   m[2]*n[4]  + m[6]*n[5]  + m[10]*n[6]  + m[14]*n[7],   m[3]*n[4]  + m[7]*n[5]  + m[11]*n[6]  + m[15]*n[7],
                 m[0]*n[8]  + m[4]*n[9]  + m[8]*n[10] + m[12]*n[11],  m[1]*n[8]  + m[5]*n[9]  + m[9]*n[10] + m[13]*n[11],  m[2]*n[8]  + m[6]*n[9]  + m[10]*n[10] + m[14]*n[11],  m[3]*n[8]  + m[7]*n[9]  + m[11]*n[10] + m[15]*n[11],
                 m[0]*n[12] + m[4]*n[13] + m[8]*n[14] + m[12]*n[15],  m[1]*n[12] + m[5]*n[13] + m[9]*n[14] + m[13]*n[15],  m[2]*n[12] + m[6]*n[13] + m[10]*n[14] + m[14]*n[15],  m[3]*n[12] + m[7]*n[13] + m[11]*n[14] + m[15]*n[15]);
  }

  template <typename T2> xvec3<T2> operator*(const xvec3<T2> &rhs) const
  {
    return xvec3(m[0]*rhs.x + m[4]*rhs.y + m[8]*rhs.z  + m[12],
                 m[1]*rhs.x + m[5]*rhs.y + m[9]*rhs.z  + m[13],
                 m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]);
  }

  template <typename T2>
  static xmat4 translation(T2 x, T2 y, T2 z) { return xmat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1); }
  template <typename T2> static xmat4 translation(const xvec3<T2> &pos) { return translation(pos.x, pos.y, pos.z); }

  template <typename T2>
  static xmat4 scale(T2 x, T2 y, T2 z) { return xmat4(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1); }
  template <typename T2> static xmat4 scale(T2 xyz) { return scale(xyz, xyz, xyz); }
  template <typename T2> static xmat4 scale(const xvec3<T2> &xyz) { return scale(xyz.x, xyz.y, xyz.z); }

  template <typename T2>
  static xmat4 rotate(T2 angleDeg, T2 x, T2 y, T2 z)
  {
    T c = static_cast<T>(cos(angleDeg / (static_cast<T2>(180) * 3.14159265358)));
    T s = static_cast<T>(sin(angleDeg / (static_cast<T2>(180) * 3.14159265358)));
    T c1 = 1 - c;

    return xmat4(x * x * c1 + c, x * y * c1 + z * s, x * z * c1 - y * s, 0,
                 x * y * c1 - z * s, y * y * c1 + c, y * z * c1 + x * s, 0,
                 x * z * c1 + y * s, y * z * c1 - x * s, z * z * c1 + c, 0,
                 0, 0, 0, 1);
  }

  template <typename T2, typename T3>
  static xmat4 rotate(T2 angleDeg, const xvec3<T3> &axis) { return rotate<T3>(angleDeg, axis.x, axis.y, axis.z); }

  template <typename T2>
  static xmat4 look_at(T2 eyeX, T2 eyeY, T2 eyeZ, T2 targetX, T2 targetY, T2 targetZ, T2 upX = 0, T2 upY = 1, T2 upZ = 0)
  {
    auto l = normalize(detail::xvec3<T2>(targetX - eyeX, targetY - eyeY, targetZ - eyeZ));
    auto s = normalize(cross(l, detail::xvec3<T2>(upX, upY, upZ)));
    auto u = cross(s, l);

    return xmat4(s.x, s.y, s.z, 0, u.x, u.y, u.z, 0, -l.x, -l.y, -l.z, 0, eyeX, eyeY, eyeZ, 1);
  }

  template <typename T2> static xmat4 perspective(T2 l, T2 r, T2 b, T2 t, T2 n, T2 f)
  {
    return xmat4(2 * n / (r - l), 0, 0, 0,
                 0, 2 * n / (t - b), 0, 0,
                 (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1,
                 0, 0, -(2 * f * n) / (f - n), 0);
  }

  template <typename T2> static xmat4 perspective(T2 fovYDeg, T2 aspectRatio, T2 nearClip, T2 farClip)
  {
    T tangent = tan((fovYDeg / 2) / (static_cast<T>(180) * static_cast<T>(3.14159265358)));
    T height = nearClip * tangent, width = height * aspectRatio;
    return perspective(-width, width, -height, height, nearClip, farClip);
  }

  template <typename T2> static xmat4 ortho(T2 l, T2 r, T2 b, T2 t, T2 n, T2 f)
  {
    return xmat4(2 / (r - l), 0, 0, 0,
                 0, 2 / (t - b), 0, 0,
                 0, 0, -2 / (f - n), 0,
                 -(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), 1);
  }

  xmat4 &invert()
  {
    T inv[16] = {
       m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10],
      -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10],
       m[1] * m[ 6] * m[15] - m[1] * m[ 7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[ 7] - m[13] * m[3] * m[ 6],
      -m[1] * m[ 6] * m[11] + m[1] * m[ 7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[ 9] * m[2] * m[ 7] + m[ 9] * m[3] * m[ 6],
      -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10],
       m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10],
      -m[0] * m[ 6] * m[15] + m[0] * m[ 7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[ 7] + m[12] * m[3] * m[ 6],
       m[0] * m[ 6] * m[11] - m[0] * m[ 7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[ 8] * m[2] * m[ 7] - m[ 8] * m[3] * m[ 6],
       m[4] * m[ 9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[ 9],
      -m[0] * m[ 9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[ 9],
       m[0] * m[ 5] * m[15] - m[0] * m[ 7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[ 7] - m[12] * m[3] * m[ 5],
      -m[0] * m[ 5] * m[11] + m[0] * m[ 7] * m[ 9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[ 9] - m[ 8] * m[1] * m[ 7] + m[ 8] * m[3] * m[ 5],
      -m[4] * m[ 9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[ 9],
       m[0] * m[ 9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[ 9],
      -m[0] * m[ 5] * m[14] + m[0] * m[ 6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[ 6] + m[12] * m[2] * m[ 5],
       m[0] * m[ 5] * m[10] - m[0] * m[ 6] * m[ 9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[ 9] + m[ 8] * m[1] * m[ 6] - m[ 8] * m[2] * m[ 5] };

    T det = 1 / (m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12]), *pm = m;
    
    *pm++ = inv[ 0] * det; *pm++ = inv[ 1] * det; *pm++ = inv[ 2] * det; *pm++ = inv[ 3] * det;
    *pm++ = inv[ 4] * det; *pm++ = inv[ 5] * det; *pm++ = inv[ 6] * det; *pm++ = inv[ 7] * det;
    *pm++ = inv[ 8] * det; *pm++ = inv[ 9] * det; *pm++ = inv[10] * det; *pm++ = inv[11] * det;
    *pm++ = inv[12] * det; *pm++ = inv[13] * det; *pm++ = inv[14] * det; *pm   = inv[15] * det;

    return *this;
  }
};

}

//---------------------------------------------------------------------------------------------------------------------
typedef detail::xvec2<float> vec2;
typedef detail::xvec2<int> ivec2;
typedef detail::xvec3<float> vec3;
typedef detail::xvec3<int> ivec3;
typedef detail::xvec4<float> vec4;
typedef detail::xvec4<int> ivec4;
typedef detail::xmat4<float> mat4;

//---------------------------------------------------------------------------------------------------------------------
template <typename T> T dot(const detail::xvec2<T> &a, const detail::xvec2<T> &b) { return a.x*b.x + a.y*b.y; }
template <typename T> T dot(const detail::xvec3<T> &a, const detail::xvec3<T> &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

//---------------------------------------------------------------------------------------------------------------------
template <typename T> detail::xvec3<T> cross(const detail::xvec3<T> &a, const detail::xvec3<T> &b)
{ return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }

//---------------------------------------------------------------------------------------------------------------------
template <typename T> T normalize(const T &vec) { return vec * (1 / vec.length()); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> T maximum(T a) { return a; }
template <typename T> T maximum(T a, T b) { return a > b ? a : b; }
template <typename Head, typename... Tail> Head maximum(Head a, Tail... b) { return maximum(a, maximum(b...)); }
template <typename T> T minimum(T a) { return a; }
template <typename T> T minimum(T a, T b) { return a < b ? a : b; }
template <typename Head, typename... Tail> Head minimum(Head a, Tail... b) { return minimum(a, minimum(b...)); }

}
