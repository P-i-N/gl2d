#pragma once

// *INDENT-OFF*
namespace gl3d::detail {

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t Dimensions> struct xmath_traits
{ using elem_type = T; static constexpr size_t dimensions = Dimensions; };

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t Dimensions> struct xvec_data { };
template <typename T> struct xvec_data<T, 2> : xmath_traits<T, 2> { union { struct { T x, y;       }; T data[2]; }; };
template <typename T> struct xvec_data<T, 3> : xmath_traits<T, 3> { union { struct { T x, y, z;    }; T data[3]; }; };
template <typename T> struct xvec_data<T, 4> : xmath_traits<T, 4> { union { struct { T x, y, z, w; }; T data[4]; }; };

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t Dimensions> struct xvec_impl : xvec_data<T, Dimensions>
{
	T &operator[](size_t index) { return &x + index; }
	const T &operator[](size_t index) const { return &x + index; }

	template <typename... Args> xvec_impl(Args&&... args) { static_assert(sizeof...(Args) == Dimensions, ""); set<0>(args...); }

	template <size_t I, typename T2> void set(T2 &&v) { data[I] = static_cast<T>(v); }
	template <size_t I, typename T2, typename... Tail> void set(T2 &&v, Tail&&... t) { set<I>(v); set<I + 1>(t...); }
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct xvec2 : xvec_impl<T, 2>
{
	xvec2() : xvec_impl(0, 0) { }
	template <class TX, class TY> xvec2(TX x, TY y) : xvec_impl((T)x, (T)y) { }
	template <class TV> xvec2(const xvec2<TV> &v): xvec_impl((T)v.x, (T)v.y) { }

	template <class T2> auto operator*(T2 scale) const { return xvec2<decltype(x * scale)>(x * scale, y * scale); }
	template <class T2> auto operator/(T2 scale) const { return xvec2<decltype(x / scale)>(x / scale, y / scale); }
	template <class T2> auto operator+(const xvec2<T2> &rhs) const { return xvec2<decltype(x + rhs.x)>(x + rhs.x, y + rhs.y); }
	template <class T2> auto operator-(const xvec2<T2> &rhs) const { return xvec2<decltype(x - rhs.x)>(x - rhs.x, y - rhs.y); }

	template <class T2> bool operator==(const xvec2<T2> &rhs) const { return x == rhs.x && y == rhs.y; }
	template <class T2> bool operator!=(const xvec2<T2> &rhs) const { return x != rhs.x || y != rhs.y; }

	T length_sq() const { return x*x + y*y; }
	T length() const { return sqrt(length_sq()); }

	static constexpr xvec2 &unit_x() { static xvec2 v(1, 0); return v; }
	static constexpr xvec2 &unit_y() { static xvec2 v(0, 1); return v; }
	static constexpr xvec2 &one()    { static xvec2 v(1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct xvec3 : xvec_impl<T, 3>
{
	xvec3() : xvec_impl(0, 0, 0) { }
	template <class TX, class TY, class TZ> xvec3(TX x, TY y, TZ z): xvec_impl((T)x, (T)y, (T)z) { }
	template <class TV> xvec3(const xvec3<TV> &v) : xvec_impl((T)v.x, (T)v.y, (T)v.z) { }
	template <class TV, class TZ> xvec3(const xvec2<TV> &v, TZ z) : xvec_impl((T)v.x, (T)v.y, (T)z) { }

	template <class T2> auto operator*(T2 scale) const { return xvec3<decltype(x * scale)>(x * scale, y * scale, z * scale); }
	template <class T2> auto operator/(T2 scale) const { return xvec3<decltype(x / scale)>(x / scale, y / scale, z / scale); }
	template <class T2> auto operator+(const xvec3<T2> &rhs) const { return xvec3<decltype(x + rhs.x)>(x + rhs.x, y + rhs.y, z + rhs.z); }
	template <class T2> auto operator-(const xvec3<T2> &rhs) const { return xvec3<decltype(x - rhs.x)>(x - rhs.x, y - rhs.y, z - rhs.z); }

	template <class T2> bool operator==(const xvec3<T2> &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	template <class T2> bool operator!=(const xvec3<T2> &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

	T length_sq() const { return x*x + y*y + z*z; }
	T length() const { return sqrt(length_sq()); }

	static constexpr xvec3 &unit_x() { static xvec3 v(1, 0, 0); return v; }
	static constexpr xvec3 &unit_y() { static xvec3 v(0, 1, 0); return v; }
	static constexpr xvec3 &unit_z() { static xvec3 v(0, 0, 1); return v; }
	static constexpr xvec3 &one()    { static xvec3 v(1, 1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct xvec4 : xvec_impl<T, 4>
{
	xvec4() : xvec_impl(0, 0, 0, 0) { }
	template <class TX, class TY, class TZ, class TW> xvec4(TX x, TY y, TZ z, TW w): xvec_impl((T)x, (T)y, (T)z, (T)w) { }
	template <class TV> xvec4(const xvec4<TV> &v): xvec_impl((T)v.x, (T)v.y, (T)v.z, (T)v.w) { }
	template <class TV, class TW> xvec4(const xvec3<TV> &v, TW w) : xvec_impl((T)v.x, (T)v.y, (T)v.z, (T)w) { }

	explicit xvec4(unsigned argb): xvec_impl(
		((argb >> 16) & 0xFFu) / 255.0f,
		((argb >> 8) & 0xFFu) / 255.0f,
		(argb & 0xFFu) / 255.0f,
		((argb >> 24) & 0xFFu) / 255.0f) { }

	template <class T2> auto operator*(T2 scale) const { return xvec4<decltype(x * scale)>(x * scale, y * scale, z * scale, w * scale); }
	template <class T2> auto operator/(T2 scale) const { return xvec4<decltype(x / scale)>(x / scale, y / scale, z / scale, w / scale); }
	template <class T2> auto operator+(const xvec4<T2> &rhs) const { return xvec4<decltype(x + rhs.x)>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	template <class T2> auto operator-(const xvec4<T2> &rhs) const { return xvec4<decltype(x - rhs.x)>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

	template <class T2> bool operator==(const xvec4<T2> &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	template <class T2> bool operator!=(const xvec4<T2> &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

	T length_sq() const { return x*x + y*y + z*z + w*w; }
	T length() const { return sqrt(length_sq()); }

	static constexpr xvec4 &unit_x() { static xvec4 v(1, 0, 0, 0); return v; }
	static constexpr xvec4 &unit_y() { static xvec4 v(0, 1, 0, 0); return v; }
	static constexpr xvec4 &unit_z() { static xvec4 v(0, 0, 1, 0); return v; }
	static constexpr xvec4 &unit_w() { static xvec4 v(0, 0, 0, 1); return v; }
	static constexpr xvec4 &one()    { static xvec4 v(1, 1, 1, 1); return v; }
	static constexpr xvec4 &red()    { static xvec4 v(1, 0, 0, 1); return v; }
	static constexpr xvec4 &green()  { static xvec4 v(0, 1, 0, 1); return v; }
	static constexpr xvec4 &blue()   { static xvec4 v(0, 0, 1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <class TV> struct basic_xbox
{
	using type = TV;
	using elem_type = typename type::elem_type;

	static const size_t dimensions = TV::dimensions;

	TV min, max;

	TV center() const { return (min + max) / 2; }
	TV size() const { max - min; }
	elem_type width() const { return max.x - min.x; }
	elem_type height() const { return max.y - min.y; }

	template <size_t I> TV corner() const { return cross_over<I>(min, max); }
	TV corner(size_t index) const
	{
		switch (index % (1 << dimensions))
		{
			case 0: return cross_over<0>(min, max);
			case 1: return cross_over<1>(min, max);
			case 2: return cross_over<2>(min, max);
			case 3: return cross_over<3>(min, max);
			case 4: return cross_over<4>(min, max);
			case 5: return cross_over<5>(min, max);
			case 6: return cross_over<6>(min, max);
			case 7: return cross_over<7>(min, max);
		}

		return min;
	}
};

//---------------------------------------------------------------------------------------------------------------------
template <class TV> using xbox2 = basic_xbox<TV>;
template <typename TV> struct xbox3 : basic_xbox<TV> { elem_type depth() const { return max.z - min.z; } };

//---------------------------------------------------------------------------------------------------------------------
template <class T, size_t Dimensions> struct xmat_data : xmath_traits<T, Dimensions>
{
	union { T m[Dimensions * Dimensions]; T data[Dimensions * Dimensions]; };
	T &operator[](size_t index) { return m[index]; }
	const T &operator[](size_t index) const { return m[index]; }
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct xmat3 : xmat_data<T, 3>
{
	xmat3()
	{
		m[0] = m[4] = m[8] = static_cast<T>(1);
		m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0;
	}

	xmat3(const xmat3 &copy) { memcpy(m, copy.m, sizeof(T) * dimensions * dimensions); }
	xmat3(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8)
	{
		auto n = m;
		*n++ = m0; *n++ = m1; *n++ = m2; *n++ = m3; *n++ = m4; *n++ = m5; *n++ = m6; *n++ = m7; *n++ = m8;
	}

	xmat3 &operator=(const xmat3 &rhs) { memcpy(m, rhs.m, sizeof(T) * 9); return *this; }

	xmat3 operator*(const xmat3 &n) const
	{
		return { m[0]*n[0] + m[3]*n[1] + m[6]*n[2], m[1]*n[0] + m[4]*n[1] + m[7]*n[2], m[2]*n[0] + m[5]*n[1] + m[8]*n[2],
		         m[0]*n[3] + m[3]*n[4] + m[6]*n[5], m[1]*n[3] + m[4]*n[4] + m[7]*n[5], m[2]*n[3] + m[5]*n[4] + m[8]*n[5],
		         m[0]*n[6] + m[3]*n[7] + m[6]*n[8], m[1]*n[6] + m[4]*n[7] + m[7]*n[8], m[2]*n[6] + m[5]*n[7] + m[8]*n[8] };
	}

	template <typename T2> xvec3<T2> operator*(const xvec3<T2> &rhs) const
	{
		return { m[0]*rhs.x + m[3]*rhs.y + m[6]*rhs.z,
		         m[1]*rhs.x + m[4]*rhs.y + m[7]*rhs.z,
		         m[2]*rhs.x + m[5]*rhs.y + m[8]*rhs.z };
	}

	template <typename TA, typename TV>
	static xmat3 make_rotation(TA angleDeg, const xvec3<TV> &axis)
	{
		T c = static_cast<T>(cos(radians(angleDeg))), s = static_cast<T>(sin(radians(angleDeg))), c1 = 1 - c;

		return { axis.x*axis.x*c1 + c,        axis.x*axis.y*c1 + axis.z*s, axis.x*axis.z*c1 - axis.y*s,
		         axis.x*axis.y*c1 - axis.z*s, axis.y*axis.y*c1 + c,        axis.y*axis.z*c1 + axis.x*s,
		         axis.x*axis.z*c1 + axis.y*s, axis.y*axis.z*c1 - axis.x*s, axis.z*axis.z*c1 + c };
	}
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> struct xmat4 : xmat_data<T, 4>
{
	xmat4()
	{
		m[0] = m[5] = m[10] = m[15] = static_cast<T>(1);
		m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0;
	}

	xmat4(const xmat4 &copy) { memcpy(m, copy.m, sizeof(T) * dimensions * dimensions); }
	xmat4(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8, T m9, T m10, T m11, T m12, T m13, T m14, T m15)
	{
		auto n = m;
		*n++ = m0; *n++ = m1; *n++ = m2; *n++ = m3; *n++ = m4; *n++ = m5; *n++ = m6; *n++ = m7;
		*n++ = m8; *n++ = m9; *n++ = m10; *n++ = m11; *n++ = m12; *n++ = m13; *n++ = m14; *n++ = m15;
	}

	xmat4(const xmat3<T> &mat3)
	{
		auto n = m; auto i = mat3.m;
		*n++ = *i++; *n++ = *i++; *n++ = *i++; *n++ = 0; *n++ = *i++; *n++ = *i++; *n++ = *i++; *n++ = 0;
		*n++ = *i++; *n++ = *i++; *n++ = *i++; *n++ = 0; *n++ = 0; *n++ = 0; *n++ = 0; *n++ = 1;
	}

	xmat4 &operator=(const xmat4 &rhs) { memcpy(m, rhs.m, sizeof(T) * 16); return *this; }

	xmat4 operator*(const xmat4 &n) const
	{
		return { m[0]*n[ 0] + m[4]*n[ 1] + m[8]*n[ 2] + m[12]*n[ 3], m[1]*n[ 0] + m[5]*n[ 1] + m[9]*n[ 2] + m[13]*n[ 3],  m[2]*n[ 0] + m[6]*n[ 1] + m[10]*n[ 2] + m[14]*n[ 3],  m[3]*n[ 0] + m[7]*n[ 1] + m[11]*n[ 2] + m[15]*n[ 3],
		         m[0]*n[ 4] + m[4]*n[ 5] + m[8]*n[ 6] + m[12]*n[ 7], m[1]*n[ 4] + m[5]*n[ 5] + m[9]*n[ 6] + m[13]*n[ 7],  m[2]*n[ 4] + m[6]*n[ 5] + m[10]*n[ 6] + m[14]*n[ 7],  m[3]*n[ 4] + m[7]*n[ 5] + m[11]*n[ 6] + m[15]*n[ 7],
		         m[0]*n[ 8] + m[4]*n[ 9] + m[8]*n[10] + m[12]*n[11], m[1]*n[ 8] + m[5]*n[ 9] + m[9]*n[10] + m[13]*n[11],  m[2]*n[ 8] + m[6]*n[ 9] + m[10]*n[10] + m[14]*n[11],  m[3]*n[ 8] + m[7]*n[ 9] + m[11]*n[10] + m[15]*n[11],
		         m[0]*n[12] + m[4]*n[13] + m[8]*n[14] + m[12]*n[15], m[1]*n[12] + m[5]*n[13] + m[9]*n[14] + m[13]*n[15],  m[2]*n[12] + m[6]*n[13] + m[10]*n[14] + m[14]*n[15],  m[3]*n[12] + m[7]*n[13] + m[11]*n[14] + m[15]*n[15] };
	}

	template <typename T2> xvec3<T2> operator*(const xvec3<T2> &rhs) const
	{
		return { m[0]*rhs.x + m[4]*rhs.y + m[ 8]*rhs.z + m[12],
		         m[1]*rhs.x + m[5]*rhs.y + m[ 9]*rhs.z + m[13],
		         m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14] };
	}

	template <typename T2> xvec4<T2> operator*(const xvec4<T2> &rhs) const
	{
		return { m[0]*rhs.x + m[4]*rhs.y + m[ 8]*rhs.z + m[12]*rhs.w,
		         m[1]*rhs.x + m[5]*rhs.y + m[ 9]*rhs.z + m[13]*rhs.w,
		         m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]*rhs.w,
		         m[3]*rhs.x + m[7]*rhs.y + m[11]*rhs.z + m[15]*rhs.w };
	}

	template <typename TV> static xmat4 make_translation(const xvec3<TV> &pos)
	{
		return { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, pos.x, pos.y, pos.z, 1 };
	}

	template <typename TV> static xmat4 make_scale(const xvec3<TV> &xyz) { return { xyz.x, 0, 0, 0, 0, xyz.y, 0, 0, 0, 0, xyz.z, 0, 0, 0, 0, 1 }; }
	template <typename TS> static xmat4 make_scale(TS xyz) { return { xyz, 0, 0, 0, 0, xyz, 0, 0, 0, 0, xyz, 0, 0, 0, 0, 1 }; }

	template <typename TA, typename TV>
	static xmat4 make_rotation(TA angleDeg, const xvec3<TV> &axis) { return xmat3<T>::make_rotation(angleDeg, axis); }

	template <typename T2>
	static xmat4 make_look_at(T2 eyeX, T2 eyeY, T2 eyeZ, T2 targetX, T2 targetY, T2 targetZ, T2 upX = 0, T2 upY = 1, T2 upZ = 0)
	{
		auto l = normalize(detail::xvec3<T2>(targetX - eyeX, targetY - eyeY, targetZ - eyeZ));
		auto s = normalize(cross(l, detail::xvec3<T2>(upX, upY, upZ)));
		auto u = cross(s, l);

		return xmat4(s.x, s.y, s.z, 0, u.x, u.y, u.z, 0, -l.x, -l.y, -l.z, 0, eyeX, eyeY, eyeZ, 1);
	}

	template <typename T2> static xmat4 make_perspective(T2 l, T2 r, T2 b, T2 t, T2 nearClip, T2 farClip)
	{
		return { 2 * nearClip / (r - l), 0, 0, 0,
		         0, 2 * nearClip / (t - b), 0, 0,
		         (r + l) / (r - l), (t + b) / (t - b), -(farClip + nearClip) / (farClip - nearClip), -1,
		         0, 0, -(2 * farClip * nearClip) / (farClip - nearClip), 0 };
	}

	template <typename T2> static xmat4 make_perspective(T2 fovYDeg, T2 aspectRatio, T2 nearClip, T2 farClip)
	{
		T tangent = tan(radians(fovYDeg / 2));
		T height = nearClip * tangent, width = height * aspectRatio;
		return make_perspective(-width, width, -height, height, nearClip, farClip);
	}

	template <typename T2> static xmat4 make_ortho(T2 l, T2 r, T2 b, T2 t, T2 nearClip, T2 farClip)
	{
		return { T(2) / T(r - l), 0, 0, 0,
		         0, T(2) / T(t - b), 0, 0,
		         0, 0, T(-2) / T(farClip - nearClip), 0,
		         T(-(r + l)) / T(r - l), T(-(t + b)) / T(t - b), T(-(farClip + nearClip)) / T(farClip - nearClip), 1 };
	}

	static xmat4 make_inverse(const xmat4 &mat)
	{
		auto m = mat.m;

		T s0 = m[0] * m[5] - m[1] * m[4];
		T s1 = m[0] * m[6] - m[2] * m[4];
		T s2 = m[0] * m[7] - m[3] * m[4];
		T s3 = m[1] * m[6] - m[2] * m[5];
		T s4 = m[1] * m[7] - m[3] * m[5];
		T s5 = m[2] * m[7] - m[3] * m[6];

		T c5 = m[10] * m[15] - m[11] * m[14];
		T c4 = m[ 9] * m[15] - m[11] * m[13];
		T c3 = m[ 9] * m[14] - m[10] * m[13];
		T c2 = m[ 8] * m[15] - m[11] * m[12];
		T c1 = m[ 8] * m[14] - m[10] * m[12];
		T c0 = m[ 8] * m[13] - m[ 9] * m[12];

		T det = 1 / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

		return { (m[ 5] * c5 - m[ 6] * c4 + m[ 7] * c3) * det,
		         (m[ 2] * c4 - m[ 1] * c5 - m[ 3] * c3) * det,
		         (m[13] * s5 - m[14] * s4 + m[15] * s3) * det,
		         (m[10] * s4 - m[ 9] * s5 - m[11] * s3) * det,
		         (m[ 6] * c2 - m[ 4] * c5 - m[ 7] * c1) * det,
		         (m[ 0] * c5 - m[ 2] * c2 + m[ 3] * c1) * det,
		         (m[14] * s2 - m[12] * s5 - m[15] * s1) * det,
		         (m[ 8] * s5 - m[10] * s2 + m[11] * s1) * det,
		         (m[ 4] * c4 - m[ 5] * c2 + m[ 7] * c0) * det,
		         (m[ 1] * c2 - m[ 0] * c4 - m[ 3] * c0) * det,
		         (m[12] * s4 - m[13] * s2 + m[15] * s0) * det,
		         (m[ 9] * s2 - m[ 8] * s4 - m[11] * s0) * det,
		         (m[ 5] * c1 - m[ 4] * c3 - m[ 6] * c0) * det,
		         (m[ 0] * c3 - m[ 1] * c1 + m[ 2] * c0) * det,
		         (m[13] * s1 - m[12] * s3 - m[14] * s0) * det,
		         (m[ 8] * s3 - m[ 9] * s1 + m[10] * s0) * det };
	}
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace gl3d {

//---------------------------------------------------------------------------------------------------------------------
static constexpr double dpi = 3.14159265358;
static constexpr double dtwo_pi = 2.0 * 3.14159265358;
static constexpr double dpi2 = 0.5 * 3.14159265358;
static constexpr double dpi4 = 0.25 * 3.14159265358;

static constexpr float pi = static_cast<float>(dpi);
static constexpr float two_pi = static_cast<float>(dtwo_pi);
static constexpr float pi2 = static_cast<float>(dpi2);
static constexpr float pi4 = static_cast<float>(dpi4);

//---------------------------------------------------------------------------------------------------------------------
template <class T> void swap(T &a, T &b) { T temp = a; a = b; b = temp; }

//---------------------------------------------------------------------------------------------------------------------
template <class T> T sign(T a) { return a ? (static_cast<T>((a > 0) ? 1 : -1)) : 0; }

//---------------------------------------------------------------------------------------------------------------------
template <class TA, class TB>
auto dot(const detail::xvec2<TA> &a, const detail::xvec2<TB> &b) { return a.x*b.x + a.y*b.y; }

template <class TA, class TB>
auto dot(const detail::xvec3<TA> &a, const detail::xvec3<TB> &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

//---------------------------------------------------------------------------------------------------------------------
template <class TA, class TB>
auto cross(const detail::xvec3<TA> &a, const detail::xvec3<TB> &b)
{ return detail::xvec3<decltype(a.x * b.x)>(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }

//---------------------------------------------------------------------------------------------------------------------
template <class TA, class TB>
auto distance_sq(const detail::xvec2<TA> &a, const detail::xvec2<TB> &b) { return (a - b).length_sq(); }

template <class TA, class TB>
auto distance_sq(const detail::xvec3<TA> &a, const detail::xvec3<TB> &b) { return (a - b).length_sq(); }

template <class TA, class TB>
auto distance(const detail::xvec2<TA> &a, const detail::xvec2<TB> &b) { return (a - b).length(); }

template <class TA, class TB>
auto distance(const detail::xvec3<TA> &a, const detail::xvec3<TB> &b) { return (a - b).length(); }

//---------------------------------------------------------------------------------------------------------------------
template <class T> T normalize(const T &vec) { return vec * (1 / vec.length()); }
template <class T> T normalize_safe(const T &vec)
{
	auto lenSq = vec.length_sq();
	if (!lenSq) return T();

	return vec * (1 / sqrt(lenSq));
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> T direction(const T &a, const T &b) { return normalize_safe(b - a); }

//---------------------------------------------------------------------------------------------------------------------
template <class T> T maximum(T a) { return a; }
template <class TA, class TB> auto maximum(TA a, TB b)
{
	using p = decltype(a + b);
	return (static_cast<p>(a) > static_cast<p>(b)) ? static_cast<p>(a) : static_cast<p>(b);
}

template <class Head, class... Tail> auto maximum(Head a, Tail... b) { return maximum(a, maximum(b...)); }

template <class T> T minimum(T a) { return a; }
template <class T> T minimum(T a, T b)
{
	using p = decltype(a + b);
	return (static_cast<p>(a) < static_cast<p>(b)) ? static_cast<p>(a) : static_cast<p>(b);
}

template <class Head, class... Tail> auto minimum(Head a, Tail... b) { return minimum(a, minimum(b...)); }

//---------------------------------------------------------------------------------------------------------------------
template <size_t I, class T> detail::xvec2<T> cross_over(const detail::xvec2<T> &a, const detail::xvec2<T> &b)
{ return { ((!I || I == 3) ? a.x : b.x), ((I / 2) ? b.y : a.y) }; };

template <size_t I, class T> detail::xvec3<T> cross_over(const detail::xvec3<T> &a, const detail::xvec3<T> &b)
{ return { ((!(I % 4) || (I % 4) == 3) ? a.x : b.x), ((I / 2) ? a.y : b.y), ((I % 4) ? a.z : b.z) }; };

//---------------------------------------------------------------------------------------------------------------------
template <class T> T degrees(T radians) { return radians / static_cast<T>(pi) * 180; }
template <class T> T radians(T degrees) { return degrees / 180 * static_cast<T>(pi); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
constexpr T align_up(T x, T alignment) { return ((x + alignment - 1) / alignment) * alignment; }

//---------------------------------------------------------------------------------------------------------------------
using vec2 = detail::xvec2<float>;
using dvec2 = detail::xvec2<double>;
using ivec2 = detail::xvec2<int>;
using uvec2 = detail::xvec2<unsigned>;
using vec3 = detail::xvec3<float>;
using dvec3 = detail::xvec3<double>;
using ivec3 = detail::xvec3<int>;
using uvec3 = detail::xvec3<unsigned>;
using vec4 = detail::xvec4<float>;
using dvec4 = detail::xvec4<double>;
using ivec4 = detail::xvec4<int>;
using uvec4 = detail::xvec4<unsigned>;
using byte_vec4 = detail::xvec4<unsigned char>;
using mat3 = detail::xmat3<float>;
using dmat3 = detail::xmat3<double>;
using mat4 = detail::xmat4<float>;
using dmat4 = detail::xmat4<double>;
using box2 = detail::xbox2<vec2>;
using ibox2 = detail::xbox2<ivec2>;
using box3 = detail::xbox3<vec3>;

}
// *INDENT-ON*
