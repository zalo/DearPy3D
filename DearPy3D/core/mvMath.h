#pragma once

#include "mvTypes.h"

static constexpr f32 MV_E        = 2.71828182f; // e
static constexpr f32 MV_LOG2E    = 1.44269504f; // log2(e)
static constexpr f32 MV_LOG10E   = 0.43429448f; // log10(e)
static constexpr f32 MV_LN2      = 0.69314718f; // ln(2)
static constexpr f32 MV_LN10     = 2.30258509f; // ln(10)
static constexpr f32 MV_PI       = 3.14159265f; // pi
static constexpr f32 MV_PI_2     = 1.57079632f; // pi/2
static constexpr f32 MV_PI_4     = 0.78539816f; // pi/4
static constexpr f32 MV_1_PI     = 0.31830988f; // 1/pi
static constexpr f32 MV_2_PI     = 0.63661977f; // 2/pi
static constexpr f32 MV_2_SQRTPI = 1.12837916f; // 2/sqrt(pi)
static constexpr f32 MV_SQRT2    = 1.41421356f; // sqrt(2)
static constexpr f32 MV_SQRT1_2  = 0.70710678f; // 1/sqrt(2)

// forward declarations
struct mvVec2;
struct mvVec3;
struct mvVec4;
struct mvMat4; // column major
struct mvTransforms;

// vector operations
mvVec2 normalize(mvVec2 v);
mvVec3 normalize(mvVec3 v);
mvVec4 normalize(mvVec4 v);
f32   length    (mvVec2 v);
f32   length    (mvVec3 v);
f32   length    (mvVec4 v);
f32   dot       (mvVec2& v1, mvVec2& v2);
f32   dot       (mvVec3& v1, mvVec3& v2);
f32   dot       (mvVec4& v1, mvVec4& v2);
mvVec3 cross    (mvVec3& v1, mvVec3& v2);

// common matrix construction
mvMat4 scale    (f32 x, f32 y, f32 z);
mvMat4 translate(f32 x, f32 y, f32 z);

// matrix operations
mvMat4 transpose(mvMat4& m);
mvMat4 invert   (mvMat4& m);
mvMat4 create_matrix(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33
);

namespace mvMath
{

	// misc. utilities
	f32 to_radians(f32 degrees);
	f32 to_degrees(f32 radians);
	f32 get_max   (f32 v1, f32 v2);
	f32 get_min   (f32 v1, f32 v2);
	f32 get_floor (f32 value);
	f32 log2      (f32 value);
	f32 square    (f32 value);
	f32 cube      (f32 value);
	f32 clamp     (f32 minV, f32 value, f32 maxV);
	f32 clamp01   (f32 value);

}

// operator overloads
mvVec2 operator+(mvVec2 left, mvVec2 right);
mvVec3 operator+(mvVec3 left, mvVec3 right);
mvVec4 operator+(mvVec4 left, mvVec4 right);

mvVec2 operator-(mvVec2 left, mvVec2 right);
mvVec3 operator-(mvVec3 left, mvVec3 right);
mvVec4 operator-(mvVec4 left, mvVec4 right);

mvVec2 operator*(mvVec2 left, mvVec2 right);
mvVec3 operator*(mvVec3 left, mvVec3 right);
mvVec4 operator*(mvVec4 left, mvVec4 right);
mvMat4 operator*(mvMat4 left, mvMat4 right);

mvMat4 operator*(mvMat4 left, f32 right);
mvMat4 operator*(f32 left, mvMat4 right);
mvVec2 operator*(mvVec2 left, f32 right);
mvVec3 operator*(mvVec3 left, f32 right);
mvVec4 operator*(mvVec4 left, f32 right);
mvVec2 operator*(f32 left, mvVec2 right);
mvVec3 operator*(f32 left, mvVec3 right);
mvVec4 operator*(f32 left, mvVec4 right);
mvVec3 operator*(mvMat4 left, mvVec3 right);
mvVec4 operator*(mvMat4 left, mvVec4 right);

mvVec2 operator/(mvVec2 left, f32 right);
mvVec3 operator/(mvVec3 left, f32 right);
mvVec4 operator/(mvVec4 left, f32 right);
mvVec2 operator/(f32 left, mvVec2 right);
mvVec3 operator/(f32 left, mvVec3 right);
mvVec4 operator/(f32 left, mvVec4 right);

struct mvVec2
{
	union { f32 x = 0.0f, r, u; };
	union { f32 y = 0.0f, g, v; };

	mvVec2() = default;

	mvVec2(f32 x, f32 y)
		: x(x), y(y)
	{
	}

	float& operator[](i32 index);
};

struct mvVec3
{
	union { f32 x = 0.0f, r; };
	union { f32 y = 0.0f, g; };
	union { f32 z = 0.0f, b; };

	mvVec3() = default;

	mvVec3(f32 x, f32 y, f32 z)
		: x(x), y(y), z(z)
	{
	}

	float& operator[](i32 index);
};

struct mvVec4
{
	union { f32 x = 0.0f, r; };
	union { f32 y = 0.0f, g; };
	union { f32 z = 0.0f, b; };
	union { f32 w = 0.0f, a; };

	mvVec4() = default;

	mvVec4(f32 x, f32 y, f32 z, f32 w)
		: x(x), y(y), z(z), w(w)
	{
	}

	operator mvVec3();

	float& operator[](i32 index);
};

struct mvMat4
{
	mvVec4 cols[4];

	mvMat4() = default;

	mvMat4(f32 value)
	{
		cols[0][0] = value;
		cols[1][1] = value;
		cols[2][2] = value;
		cols[3][3] = value;
	}

	mvMat4(mvVec4 c0, mvVec4 c1, mvVec4 c2, mvVec4 c3)
	{
		cols[0] = c0;
		cols[1] = c1;
		cols[2] = c2;
		cols[3] = c3;
	}

	mvVec4& operator[](i32 index);
	f32& at(i32 index);
};

struct mvTransforms
{
	mvMat4 model = mvMat4(1.0f);
	mvMat4 modelView = mvMat4(1.0f);
	mvMat4 modelViewProjection = mvMat4(1.0f);
};

mvMat4 look_at(mvVec3 eye, mvVec3 center, mvVec3 up); // world space eye, center, up
mvMat4 fps_view(mvVec3 eye, f32 pitch, float yaw);   // world space eye, center, up
mvMat4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar);
mvMat4 perspective(f32 fovy, f32 aspect, f32 zNear, f32 zFar);
