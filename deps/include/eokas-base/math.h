
#ifndef  _EOKAS_BASE_MATH_H_
#define  _EOKAS_BASE_MATH_H_

#include "header.h"

_BeginNamespace(eokas)

/**
 * Vector2
*/
class Vector2
{
public:
	static const Vector2 zero;
	static const Vector2 one;
	static const Vector2 up;
	static const Vector2 down;	
	static const Vector2 right;
	static const Vector2 left;

public:
    static Vector2 random(f32_t scale = 1);

public:
	Vector2();
	Vector2(f32_t x, f32_t y);
	Vector2(const f32_t (&v)[2]);
	Vector2(const Vector2& v);
	~Vector2();

public:
	Vector2& operator=(const Vector2& v);
	Vector2 operator-() const;

	Vector2 operator+(const Vector2& v) const;
	Vector2 operator-(const Vector2& v) const;
	Vector2 operator*(const Vector2& v) const;
	Vector2 operator*(f32_t t) const;

	Vector2& operator+=(const Vector2& v);
	Vector2& operator-=(const Vector2& v);
	Vector2& operator*=(const Vector2& v);
	Vector2& operator*=(f32_t t);

	bool operator==(const Vector2& v) const;
	bool operator!=(const Vector2& v) const;

public:
	f32_t sqrmagnitude() const;
	f32_t magnitude() const;
	Vector2 normalized() const;
	Vector2& normalize();

public:
	f32_t x;
	f32_t y;
};

/**
 * Vector3
*/
class Vector3
{
public:
	static const Vector3 zero;
	static const Vector3 one;
	static const Vector3 up;
	static const Vector3 down;
	static const Vector3 right;
	static const Vector3 left;
	static const Vector3 forward;
	static const Vector3 back;

public:
    static Vector3 random(f32_t scale = 1);

public:
	Vector3();
	Vector3(f32_t x, f32_t y, f32_t z);
	Vector3(const f32_t (&v)[3]);
	Vector3(const Vector2& v);
	Vector3(const Vector3& v);
	~Vector3();

public:
	Vector3& operator=(const Vector3& v);
	Vector3 operator-() const;

	Vector3 operator+(const Vector3& v) const;
	Vector3 operator-(const Vector3& v) const;
	Vector3 operator*(const Vector3& v) const;
	Vector3 operator*(f32_t t) const;

	Vector3& operator+=(const Vector3& v);
	Vector3& operator-=(const Vector3& v);
	Vector3& operator*=(const Vector3& v);
	Vector3& operator*=(f32_t t);

	bool operator==(const Vector3& v) const;
	bool operator!=(const Vector3& v) const;

public:
	f32_t sqrmagnitude() const;
	f32_t magnitude() const;
	Vector3 normalized() const;
	Vector3& normalize();

public:
	f32_t x;
	f32_t y;
	f32_t z;
};

/**
 * Vector4
*/
class Vector4
{
public:
	static const Vector4 zero;
	static const Vector4 one;

public:
	Vector4();
	Vector4(f32_t x, f32_t y, f32_t z, f32_t w = 1);
	Vector4(const f32_t (&v)[4]);
	Vector4(const Vector3& v);
	Vector4(const Vector4& v);
	~Vector4();

public:
	Vector4& operator=(const Vector4& v);
	Vector4 operator-() const;

	Vector4 operator+(const Vector4& v) const;
	Vector4 operator-(const Vector4& v) const;
	Vector4 operator*(const Vector4& v) const;
	Vector4 operator*(f32_t t) const;

	Vector4& operator+=(const Vector4& v);
	Vector4& operator-=(const Vector4& v);
	Vector4& operator*=(const Vector4& v);
	Vector4& operator*=(f32_t t);

	bool operator==(const Vector4& v) const;
	bool operator!=(const Vector4& v) const;

public:
	f32_t sqrmagnitude() const;
	f32_t magnitude() const;
	Vector4 normalized() const;
	Vector4& normalize();

public:
	f32_t x;
	f32_t y;
	f32_t z;
	f32_t w;
};

/**
 * 3x3 Matrix
*/
class Matrix3
{
public:
	static const Matrix3 identity;

public:
	Matrix3();
	Matrix3(
		f32_t _00, f32_t _01, f32_t _02,
		f32_t _10, f32_t _11, f32_t _12,
		f32_t _20, f32_t _21, f32_t _22);
	Matrix3(const f32_t (&m)[3][3]);
	Matrix3(const Matrix3& m);
	~Matrix3();

public:
	Matrix3& operator=(const Matrix3& m);
	Matrix3 operator-() const;

	Matrix3 operator+(const Matrix3& m) const;
	Matrix3 operator-(const Matrix3& m) const;
	Matrix3 operator*(const Matrix3& m) const;

	Matrix3& operator+=(const Matrix3& m);
	Matrix3& operator-=(const Matrix3& m);
	Matrix3& operator*=(const Matrix3& m);

	bool operator==(const Matrix3& m) const;
	bool operator!=(const Matrix3& m) const;

public:
	f32_t determinant() const;
	Matrix3 transposed() const;
	Matrix3& transpose();

public:
	f32_t value[3][3];
};

/**
 * 4x4 Matrix
*/
class Matrix4
{
public:
	static const Matrix4 identity;

public:
	Matrix4();
	Matrix4(
		f32_t _00, f32_t _01, f32_t _02, f32_t _03,
		f32_t _10, f32_t _11, f32_t _12, f32_t _13,
		f32_t _20, f32_t _21, f32_t _22, f32_t _23,
		f32_t _30, f32_t _31, f32_t _32, f32_t _33);
	Matrix4(const f32_t (&m)[4][4]);
	Matrix4(const Matrix4& m);
	Matrix4(const Matrix3& m);
	~Matrix4();

public:
	Matrix4& operator=(const Matrix4& m);
	Matrix4 operator-() const;

	Matrix4 operator+(const Matrix4& m) const;
	Matrix4 operator-(const Matrix4& m) const;
	Matrix4 operator*(const Matrix4& m) const;

	Matrix4& operator+=(const Matrix4& m);
	Matrix4& operator-=(const Matrix4& m);
	Matrix4& operator*=(const Matrix4& m);

	bool operator==(const Matrix4& m) const;
	bool operator!=(const Matrix4& m) const;

public:
	float determinant() const;
	Matrix4 transposed() const;
	Matrix4& transpose();

public:
	f32_t value[4][4];
};


/**
 * Quaternion
*/
class Quaternion
{
public:
	static const Quaternion identity;

public:
	Quaternion();
	Quaternion(f32_t x, f32_t y, f32_t z, f32_t w = 1);
	Quaternion(const f32_t (&q)[4]);
	Quaternion(const Quaternion& q);
	~Quaternion();

public:
	Quaternion& operator=(const Quaternion& q);
	Quaternion operator-() const;

	Quaternion operator+(const Quaternion& q) const;
	Quaternion operator-(const Quaternion& q) const;
	Quaternion operator*(const Quaternion& q) const;

	Quaternion& operator+=(const Quaternion& q);
	Quaternion& operator-=(const Quaternion& q);
	Quaternion& operator*=(const Quaternion& q);

	bool operator==(const Quaternion& q) const;
	bool operator!=(const Quaternion& q) const;

	f32_t sqrmagnitude() const;
	f32_t magnitude() const;
	Quaternion conjugate() const;
	Quaternion inverse() const;

public:
	f32_t x;
	f32_t y;
	f32_t z;
	f32_t w;
};

/**
 * Spherical coords
*/
class Spherical
{
public:
    Spherical();
    Spherical(f32_t radius, f32_t phi, f32_t theta);
    Spherical(const Vector3& point);
    Spherical(const Spherical& other);
    ~Spherical();

public:
    operator Vector3();

public:
    f32_t radius;
    f32_t phi;
    f32_t theta;
};

/**
 * Ray
*/
class Ray
{
public:
	Ray();
	Ray(const Vector3& origin, const Vector3& diBounds2ion);
	Ray(const Ray& other);
	~Ray();

public:
	Vector3 point(f32_t t) const;

public:
	Vector3 origin;
	Vector3 diBounds2ion;
};

/**
 * Plane
*/
class Plane
{
public:
	Plane();
	Plane(const Vector3& normal, f32_t distance);
	Plane(const Vector3& normal, const Vector3& position);
	Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3);
	Plane(const Plane& other);
	~Plane();

public:
	f32_t side(const Vector3& p) const;

public:
	Vector3 normal;
	f32_t distance;
};

/**
 * Sphere
*/
class Sphere
{
public:
	Sphere();
	Sphere(f32_t x, f32_t y, f32_t z, f32_t r);
	Sphere(const Vector3& o, f32_t r);
	Sphere(const Sphere& other);
	~Sphere();

public:
	f32_t side(const Vector3& p) const;
    bool include(const Vector3& p) const;
    Sphere& expand(const Vector3&);

public:
	Vector3 origin;
	f32_t radius;
};

/**
 * Bounds2
*/
class Bounds2
{
public:
	Bounds2(const Vector2& center, const Vector2& extent);
	Bounds2(const Bounds2& other);
	~Bounds2();

public:
	f32_t width() const;
	f32_t height() const;
	Vector2 center() const;
	Vector2 extent() const;	
	bool contains(const Vector2& p) const;
	Bounds2& expand(const Vector2& p);
	Bounds2& expand(const Bounds2& b);

public:
	Vector2 min;
	Vector2 max;
};

/**
 * Bounds3
*/
class Bounds3
{
public:
	Bounds3(const Vector3& center, const Vector3& extent);
	Bounds3(const Bounds3& other);
	~Bounds3();

public:
	f32_t width() const;
	f32_t height() const;
	f32_t depth() const;
	Vector3 center() const;
	Vector3 extent() const;	
	bool contains(const Vector3& p) const;
	Bounds3& expand(const Vector3& p);
	Bounds3& expand(const Bounds3& b);

public:
	Vector3 min;
	Vector3 max;
};

/**
 * Math
*/
class Math
{
public:
	static const f32_t EPSILON_4;
	static const f32_t EPSILON_5;
	static const f32_t EPSILON_6;
	static const f32_t PI;
    static const f32_t PI_MUL_2;
    static const f32_t PI_DIV_2;
    static const f32_t INV_PI;
    static const f32_t SQRT_2;
    static const f32_t SQRT_3;

	static const f32_t DEGREES_PER_RADIAN;
	static const f32_t RADIANS_PER_DEGREE;

public:
	static void fill(f32_t (&target)[2], f32_t x, f32_t y);
	static void fill(f32_t (&target)[3], f32_t x, f32_t y, f32_t z);
	static void fill(f32_t (&target)[4], f32_t x, f32_t y, f32_t z, f32_t w);

	static f32_t radianToAngle(f32_t radian);
	static f32_t angleToRadian(f32_t degree);

	static bool between(f32_t x, f32_t a, f32_t b);
	static bool between(const Vector2& x, const Vector2& a, const Vector2& b);
	static bool between(const Vector3& x, const Vector3& a, const Vector3& b);
	
	static f32_t clamp(f32_t x, f32_t a, f32_t b);
	static Vector2 clamp(const Vector2& x, const Vector2& a, const Vector2& b);
	static Vector3 clamp(const Vector3& x, const Vector3& a, const Vector3& b);

	static f32_t lerp(f32_t a, f32_t b, f32_t t);
	static Vector2 lerp(const Vector2& a, const Vector2& b, f32_t t);
	static Vector3 lerp(const Vector3& a, const Vector3& b, f32_t t);
	static Vector2 lerp(const Vector2& a, const Vector2& b, const Vector2& t);
	static Vector3 lerp(const Vector3& a, const Vector3& b, const Vector3& t);
	static Quaternion lerp(const Quaternion& a, const Quaternion& b, f32_t t);

	static Vector3 slerp(const Vector3& a, const Vector3& b, f32_t t);
	static Quaternion slerp(const Quaternion& a, const Quaternion& b, f32_t t);

	// a * sin(w * x + q) + k
	// a * cos(w * x + q) + k
	static f32_t sampleSinCurve(f32_t a, f32_t w, f32_t q, f32_t k, f32_t min, f32_t max, f32_t t);
	static f32_t sampleCosCurve(f32_t a, f32_t w, f32_t q, f32_t k, f32_t min, f32_t max, f32_t t);

	static Vector3 sampleBezierCurve(const Vector3 p0, const Vector3 p1, const Vector3& p2, f32_t t);
	static Vector3 sampleBezierCurve(const Vector3 p0, const Vector3 p1, const Vector3& p2, const Vector3& p3, f32_t t);
	
public:
	static Vector3 cross(const Vector3& v1, const Vector3& v2);
	static f32_t dot(const Vector2& v1, const Vector2& v2);
	static f32_t dot(const Vector3& v1, const Vector3& v2);
	static f32_t dot(const Vector4& v1, const Vector4& v2);

	static f32_t cosA(const Vector2& v1, const Vector2& v2);
	static f32_t cosA(const Vector3& v1, const Vector3& v2);
	static f32_t radians(const Vector2& v1, const Vector2& v2);
	static f32_t radians(const Vector3& v1, const Vector3& v2);
	static f32_t angles(const Vector2& v1, const Vector2& v2);
	static f32_t angles(const Vector3& v1, const Vector3& v2);

public:
	static bool intersects(const Ray& ray, const Plane& plane);
	static bool intersects(const Ray& ray, const Sphere& sphere);
	static bool intersects(const Ray& ray, const Bounds3& bounds);
	static bool intersects(f32_t& t, const Ray& ray, const Plane& plane);
	static bool intersects(f32_t& t, const Ray& ray, const Sphere& sphere);
	static bool intersects(f32_t& t, const Ray& ray, const Bounds3& bounds);
	static bool intersects(f32_t& t, const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3 v2);
	
public:
	static void transform(Vector3& result, const Vector3& v, const Matrix3& m);
	static void transform(Vector4& result, const Vector4& v, const Matrix4& m);
	static void transform(Matrix3& result, const Matrix3& m1, const Matrix3& m2);
	static void transform(Matrix4& result, const Matrix4& m1, const Matrix4& m2);

public:
	static Matrix3 matrixTranslation(const Vector2& dir);
	static Matrix4 matrixTranslation(const Vector3& dir);

	static Matrix3 matrixRotation(const Vector2& axis, f32_t angle);
	static Matrix4 matrixRotation(const Vector3& axis, f32_t angle);
	
	static Matrix3 matrixScaling(const Vector2& scale);
	static Matrix4 matrixScaling(const Vector3& scale);

	static Matrix4 matrixLookToLH(const Vector3& pos, const Vector3& forward, const Vector3& up);
	static Matrix4 matrixLookAtLH(const Vector3& pos, const Vector3& focus, const Vector3& up);
	static Matrix4 matrixLookToRH(const Vector3& pos, const Vector3& forward, const Vector3& up);
	static Matrix4 matrixLookAtRH(const Vector3& pos, const Vector3& focus, const Vector3& up);

	static Matrix4 matrixOrthographicLH(f32_t width, f32_t height, f32_t near, f32_t far);
	static Matrix4 matrixOrthographicRH(f32_t width, f32_t height, f32_t near, f32_t far);
	static Matrix4 matrixOrthographicOffCenterLH(f32_t left, f32_t right, f32_t top, f32_t bottom, f32_t near, f32_t far);
	static Matrix4 matrixOrthographicOffCenterRH(f32_t left, f32_t right, f32_t top, f32_t bottom, f32_t near, f32_t far);

	static Matrix4 matrixPerspectiveLH(f32_t width, f32_t height, f32_t near, f32_t far);
	static Matrix4 matrixPerspectiveRH(f32_t width, f32_t height, f32_t near, f32_t far);
	static Matrix4 matrixPerspectiveFovLH(f32_t fov, f32_t aspect, f32_t near, f32_t far);
	static Matrix4 matrixPerspectiveFovRH(f32_t fov, f32_t aspect, f32_t near, f32_t far);
	static Matrix4 matrixPerspectiveOffCenterLH(f32_t left, f32_t right, f32_t top, f32_t bottom, f32_t near, f32_t far);
	static Matrix4 matrixPerspectiveOffCenterRH(f32_t left, f32_t right, f32_t top, f32_t bottom, f32_t near, f32_t far);

	static Quaternion quaternionFromEulerAngles(f32_t x, f32_t y, f32_t z);
	static Quaternion quaternionFromAxisAngle(const Vector3& axis, f32_t angle);
	static Quaternion quaternionBetween(const Vector3& a, const Vector3& b);
	static Quaternion quaternionBetween(const Quaternion& a, const Quaternion& b);
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_MATH_H_
