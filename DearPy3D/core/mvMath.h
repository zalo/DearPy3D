#pragma once

#include "mvContext.h"

mv_global constexpr float PI = 3.14159265f;
mv_global constexpr double PI_D = 3.1415926535897932;

struct mvVec2
{
	union { float x, r, u; };
	union { float y, g, v; };

	float& operator[](int index);
};

struct mvVec3
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };

	float& operator[](int index);
};

struct mvVec4
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };
	union { float w, a; };

	float& operator[](int index);
	mvVec3 xyz()
	{
		return mvVec3{ x, y, z };
	}
};

struct mvMat4
{
	mvVec4 cols[4];

	mvVec4& operator[](int index);
};

mvMat4 mvIdentityMat4();

struct mvTransforms
{
	mvMat4 model = mvIdentityMat4();
	mvMat4 modelView = mvIdentityMat4();
	mvMat4 modelViewProjection = mvIdentityMat4();
};

mvVec2 operator+(mvVec2& left, mvVec2& right);
mvVec2 operator-(mvVec2& left, mvVec2& right);
mvVec2 operator*(mvVec2& left, mvVec2& right);
mvVec2 operator*(mvVec2& left, float right);

mvVec3 operator+(mvVec3& left, mvVec3& right);
mvVec3 operator-(mvVec3& left, mvVec3& right);
mvVec3 operator*(mvVec3& left, mvVec3& right);
mvVec3 operator*(mvVec3& left, float right);

mvVec4 operator+(mvVec4& left, mvVec4& right);
mvVec4 operator-(mvVec4& left, mvVec4& right);
mvVec4 operator*(mvVec4& left, mvVec4& right);
mvVec4 operator*(mvVec4& left, float right);
mvVec4 operator*(mvMat4& left, mvVec4& right);

mvMat4 operator*(mvMat4& left, mvMat4& right);

mvVec3 mvNormalize(mvVec3& v);
mvVec3 mvCross    (mvVec3& v1, mvVec3& v2);
float  mvDot       (mvVec3& v1, mvVec3& v2);
float  mvRadians   (float degrees);

mvMat4 mvYawPitchRoll (float yaw, float pitch, float roll);
mvMat4 mvLookAtLH     (mvVec3& eye, mvVec3& center, mvVec3& up);
mvMat4 mvLookAtRH     (mvVec3& eye, mvVec3& center, mvVec3& up);
mvMat4 mvTranslate    (mvMat4& m, mvVec3& v);
mvMat4 mvRotate       (mvMat4& m, float angle, mvVec3& v);
mvMat4 mvScale        (mvMat4& m, mvVec3& v);
mvMat4 mvOrthoLH      (float left, float right, float bottom, float top, float zNear, float zFar);
mvMat4 mvPerspectiveLH(float fovy, float aspect, float zNear, float zFar);
mvMat4 mvPerspectiveRH(float fovy, float aspect, float zNear, float zFar);