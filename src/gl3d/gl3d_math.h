#pragma once

namespace gl3d::detail {

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct precision_rank { static constexpr int value = 0; };
template <> struct precision_rank<int> { static constexpr int value = 1; };
template <> struct precision_rank<float> { static constexpr int value = 2; };
template <> struct precision_rank<double> { static constexpr int value = 3; };
template <> struct precision_rank<long double> { static constexpr int value = 4; };

//---------------------------------------------------------------------------------------------------------------------
template <class TA, class TB, bool> struct precision_filter { using best = TA; using worst = TB; };
template <class TA, class TB> struct precision_filter<TA, TB, false> { using best = TB; using worst = TA; };

template <class TA, class TB>
using precision = typename precision_filter<TA, TB, precision_rank<TA>::value >= precision_rank<TB>::value>::best;

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t Dimensions> struct xmath_traits { typedef T elem_type; static const size_t dimensions = Dimensions; };

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t Dimensions> struct xvec_data { };
template <typename T> struct xvec_data<T, 2> : xmath_traits<T, 2> { union { struct { T x, y; }; T data[2]; }; };
template <typename T> struct xvec_data<T, 3> : xmath_traits<T, 3> { union { struct { T x, y, z; }; T data[3]; }; };
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
	
	T length_sq() const { return x*x + y*y; }
	T length() const { return sqrt(length_sq()); }

	static const xvec2 &unit_x() { static xvec2 v(1, 0); return v; }
	static const xvec2 &unit_y() { static xvec2 v(0, 1); return v; }
	static const xvec2 &one()    { static xvec2 v(1, 1); return v; }
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

	T length_sq() const { return x*x + y*y + z*z; }
	T length() const { return sqrt(length_sq()); }

	static const xvec3 &unit_x() { static xvec3 v(1, 0, 0); return v; }
	static const xvec3 &unit_y() { static xvec3 v(0, 1, 0); return v; }
	static const xvec3 &unit_z() { static xvec3 v(0, 0, 1); return v; }
	static const xvec3 &one()    { static xvec3 v(1, 1, 1); return v; }
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> struct xvec4 : xvec_impl<T, 4>
{
	xvec4() : xvec_impl(0, 0, 0, 0) { }
	template <class TX, class TY, class TZ, class TW> xvec4(TX x, TY y, TZ z, TW w): xvec_impl((T)x, (T)y, (T)z, (T)w) { }
	template <class TV> xvec4(const xvec4<TV> &v): xvec_impl((T)v.x, (T)v.y, (T)v.z, (T)v.w) { }
	template <class TV, class TW> xvec4(const xvec3<TV> &v, TW w) : xvec_impl((T)v.x, (T)v.y, (T)v.z, (T)w) { }

	explicit xvec4(uint32_t argb): xvec_impl(
		((argb >> 16) & 0xFFu) / 255.0f,
		((argb >> 8) & 0xFFu) / 255.0f,
		(argb & 0xFFu) / 255.0f,
		((argb >> 24) & 0xFFu) / 255.0f) { }

	template <class T2> auto operator*(T2 scale) const { return xvec4<decltype(x * scale)>(x * scale, y * scale, z * scale, w * scale); }
	template <class T2> auto operator/(T2 scale) const { return xvec4<decltype(x / scale)>(x / scale, y / scale, z / scale, w / scale); }
	template <class T2> auto operator+(const xvec4<T2> &rhs) const { return xvec4<decltype(x + rhs.x)>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	template <class T2> auto operator-(const xvec4<T2> &rhs) const { return xvec4<decltype(x - rhs.x)>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

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
template <class TV> struct basic_xbox
{
	typedef TV type;
	typedef typename TV::elem_type elem_type;
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
		return xmat3(m[0]*n[0] + m[3]*n[1] + m[6]*n[2], m[1]*n[0] + m[4]*n[1] + m[7]*n[2], m[2]*n[0] + m[5]*n[1] + m[8]*n[2],
								 m[0]*n[3] + m[3]*n[4] + m[6]*n[5], m[1]*n[3] + m[4]*n[4] + m[7]*n[5], m[2]*n[3] + m[5]*n[4] + m[8]*n[5],
								 m[0]*n[6] + m[3]*n[7] + m[6]*n[8], m[1]*n[6] + m[4]*n[7] + m[7]*n[8], m[2]*n[6] + m[5]*n[7] + m[8]*n[8]);
	}
	
	template <typename T2> xvec3<T2> operator*(const xvec3<T2> &rhs) const
	{
		return xvec3<T2>(m[0]*rhs.x + m[3]*rhs.y + m[6]*rhs.z,
										 m[1]*rhs.x + m[4]*rhs.y + m[7]*rhs.z,
										 m[2]*rhs.x + m[5]*rhs.y + m[8]*rhs.z);
	}

	template <typename TA, typename TV>
	static xmat3 make_rotation(TA angleDeg, const xvec3<TV> &axis)
	{
		T c = static_cast<T>(cos(radians(angleDeg))), s = static_cast<T>(sin(radians(angleDeg))), c1 = 1 - c;

		return xmat3(axis.x * axis.x * c1 + c, axis.x * axis.y * c1 + axis.z * s, axis.x * axis.z * c1 - axis.y * s,
		             axis.x * axis.y * c1 - axis.z * s, axis.y * axis.y * c1 + c, axis.y * axis.z * c1 + axis.x * s,
		             axis.x * axis.z * c1 + axis.y * s, axis.y * axis.z * c1 - axis.x * s, axis.z * axis.z * c1 + c);
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
		return xmat4(m[0]*n[ 0] + m[4]*n[ 1] + m[8]*n[ 2] + m[12]*n[ 3], m[1]*n[ 0] + m[5]*n[ 1] + m[9]*n[ 2] + m[13]*n[ 3],  m[2]*n[ 0] + m[6]*n[ 1] + m[10]*n[ 2] + m[14]*n[ 3],  m[3]*n[ 0] + m[7]*n[ 1] + m[11]*n[ 2] + m[15]*n[ 3],
								 m[0]*n[ 4] + m[4]*n[ 5] + m[8]*n[ 6] + m[12]*n[ 7], m[1]*n[ 4] + m[5]*n[ 5] + m[9]*n[ 6] + m[13]*n[ 7],  m[2]*n[ 4] + m[6]*n[ 5] + m[10]*n[ 6] + m[14]*n[ 7],  m[3]*n[ 4] + m[7]*n[ 5] + m[11]*n[ 6] + m[15]*n[ 7],
								 m[0]*n[ 8] + m[4]*n[ 9] + m[8]*n[10] + m[12]*n[11], m[1]*n[ 8] + m[5]*n[ 9] + m[9]*n[10] + m[13]*n[11],  m[2]*n[ 8] + m[6]*n[ 9] + m[10]*n[10] + m[14]*n[11],  m[3]*n[ 8] + m[7]*n[ 9] + m[11]*n[10] + m[15]*n[11],
								 m[0]*n[12] + m[4]*n[13] + m[8]*n[14] + m[12]*n[15], m[1]*n[12] + m[5]*n[13] + m[9]*n[14] + m[13]*n[15],  m[2]*n[12] + m[6]*n[13] + m[10]*n[14] + m[14]*n[15],  m[3]*n[12] + m[7]*n[13] + m[11]*n[14] + m[15]*n[15]);
	}

	template <typename T2> xvec3<T2> operator*(const xvec3<T2> &rhs) const
	{
		return xvec3<T2>(m[0]*rhs.x + m[4]*rhs.y + m[ 8]*rhs.z + m[12],
										 m[1]*rhs.x + m[5]*rhs.y + m[ 9]*rhs.z + m[13],
										 m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]);
	}

	template <typename T2> xvec4<T2> operator*(const xvec4<T2> &rhs) const
	{
		return xvec4<T2>(m[0]*rhs.x + m[4]*rhs.y + m[ 8]*rhs.z + m[12]*rhs.w,
										 m[1]*rhs.x + m[5]*rhs.y + m[ 9]*rhs.z + m[13]*rhs.w,
										 m[2]*rhs.x + m[6]*rhs.y + m[10]*rhs.z + m[14]*rhs.w,
										 m[3]*rhs.x + m[7]*rhs.y + m[11]*rhs.z + m[15]*rhs.w);
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
		return xmat4(2 * nearClip / (r - l), 0, 0, 0,
								 0, 2 * nearClip / (t - b), 0, 0,
								 (r + l) / (r - l), (t + b) / (t - b), -(farClip + nearClip) / (farClip - nearClip), -1,
								 0, 0, -(2 * farClip * nearClip) / (farClip - nearClip), 0);
	}

	template <typename T2> static xmat4 make_perspective(T2 fovYDeg, T2 aspectRatio, T2 nearClip, T2 farClip)
	{
		T tangent = tan(radians(fovYDeg / 2));
		T height = nearClip * tangent, width = height * aspectRatio;
		return make_perspective(-width, width, -height, height, nearClip, farClip);
	}

	template <typename T2> static xmat4 make_ortho(T2 l, T2 r, T2 b, T2 t, T2 nearClip, T2 farClip)
	{
		return xmat4(2 / (r - l), 0, 0, 0,
								 0, 2 / (t - b), 0, 0,
								 0, 0, -2 / (farClip - nearClip), 0,
								 -(r + l) / (r - l), -(t + b) / (t - b), -(farClip + nearClip) / (farClip - nearClip), 1);
	}

	xmat4 &invert()
	{
		T inv[16] = {
			 m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10],
			-m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10],
			 m[1]*m[ 6]*m[15] - m[1]*m[ 7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[ 7] - m[13]*m[3]*m[ 6],
			-m[1]*m[ 6]*m[11] + m[1]*m[ 7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[ 9]*m[2]*m[ 7] + m[ 9]*m[3]*m[ 6],
			-m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10],
			 m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10],
			-m[0]*m[ 6]*m[15] + m[0]*m[ 7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[ 7] + m[12]*m[3]*m[ 6],
			 m[0]*m[ 6]*m[11] - m[0]*m[ 7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[ 8]*m[2]*m[ 7] - m[ 8]*m[3]*m[ 6],
			 m[4]*m[ 9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[ 9],
			-m[0]*m[ 9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[ 9],
			 m[0]*m[ 5]*m[15] - m[0]*m[ 7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[ 7] - m[12]*m[3]*m[ 5],
			-m[0]*m[ 5]*m[11] + m[0]*m[ 7]*m[ 9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[ 9] - m[ 8]*m[1]*m[ 7] + m[ 8]*m[3]*m[ 5],
			-m[4]*m[ 9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[ 9],
			 m[0]*m[ 9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[ 9],
			-m[0]*m[ 5]*m[14] + m[0]*m[ 6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[ 6] + m[12]*m[2]*m[ 5],
			 m[0]*m[ 5]*m[10] - m[0]*m[ 6]*m[ 9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[ 9] + m[ 8]*m[1]*m[ 6] - m[ 8]*m[2]*m[ 5] };

		T det = 1 / (m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12]), *pm = m;

		*pm++ = inv[ 0]*det; *pm++ = inv[ 1]*det; *pm++ = inv[ 2]*det; *pm++ = inv[ 3]*det;
		*pm++ = inv[ 4]*det; *pm++ = inv[ 5]*det; *pm++ = inv[ 6]*det; *pm++ = inv[ 7]*det;
		*pm++ = inv[ 8]*det; *pm++ = inv[ 9]*det; *pm++ = inv[10]*det; *pm++ = inv[11]*det;
		*pm++ = inv[12]*det; *pm++ = inv[13]*det; *pm++ = inv[14]*det; *pm   = inv[15]*det;

		return *this;
	}

	xmat4 &transpose()
	{
		swap(m[1], m[4]); swap(m[2], m[ 8]); swap(m[ 3], m[12]);
		swap(m[6], m[9]); swap(m[7], m[13]); swap(m[11], m[14]);

		return *this;
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
	using p = detail::precision<TA, TB>;
	return (static_cast<p>(a) > static_cast<p>(b)) ? static_cast<p>(a) : static_cast<p>(b);
}

template <class Head, class... Tail> auto maximum(Head a, Tail... b) { return maximum(a, maximum(b...)); }

template <class T> T minimum(T a) { return a; }
template <class T> T minimum(T a, T b)
{
	using p = detail::precision<TA, TB>;
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
typedef detail::xvec2<float> vec2;
typedef detail::xvec2<double> dvec2;
typedef detail::xvec2<int> ivec2;
typedef detail::xvec3<float> vec3;
typedef detail::xvec3<double> dvec3;
typedef detail::xvec3<int> ivec3;
typedef detail::xvec4<float> vec4;
typedef detail::xvec4<double> dvec4;
typedef detail::xvec4<int> ivec4;
typedef detail::xmat3<float> mat3;
typedef detail::xmat3<double> dmat3;
typedef detail::xmat4<float> mat4;
typedef detail::xmat4<double> dmat4;
typedef detail::xbox2<vec2> box2;
typedef detail::xbox2<ivec2> ibox2;
typedef detail::xbox3<vec3> box3;

}
