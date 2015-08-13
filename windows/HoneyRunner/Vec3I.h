// Vec3I.h
#pragma once

struct Vec3I
{
	int x;
	int y;
	int z;

	inline bool operator ==(const Vec3I& vec) const
	{
		return x == vec.x && y == vec.y && z == vec.z;
	}

	inline bool operator !=(const Vec3I& vec) const
	{
		return x != vec.x || y != vec.y || z != vec.z;
	}

	inline void operator +=(const Vec3I& vec)
	{
		x += vec.x; 
		y += vec.y; 
		z += vec.z; 
	}

	inline Vec3I operator -() const
	{
		Vec3I retval;

		retval.x = -x; 
		retval.y = -y; 
		retval.z = -z; 

		return retval;
	}

	inline Vec3I operator +(const Vec3I& vec) const
	{
		Vec3I retval;

		retval.x = x + vec.x; 
		retval.y = y + vec.y; 
		retval.z = z + vec.z; 

		return retval;
	}

	inline void operator -=(const Vec3I& vec)
	{
		x -= vec.x; 
		y -= vec.y; 
		z -= vec.z; 
	}

	inline Vec3I operator -(const Vec3I& vec) const
	{
		Vec3I retval;

		retval.x = x - vec.x; 
		retval.y = y - vec.y; 
		retval.z = z - vec.z; 

		return retval;
	}

	inline void operator *=(int f)
	{
		x *= f; 
		y *= f; 
		z *= f; 
	}

	Vec3I operator *(int f) const
	{
		Vec3I retval;

		retval.x = x * f; 
		retval.y = y * f; 
		retval.z = z * f; 

		return retval;
	}

	inline void operator /=(int f)
	{
		x /= f; 
		y /= f; 
		z /= f; 
	}

	inline Vec3I operator /(int f) const
	{
		Vec3I retval;

		retval.x = x / f; 
		retval.y = y / f; 
		retval.z = z / f; 

		return retval;
	}
};

extern const Vec3I V3I0;
extern const Vec3I V3I1;
extern const Vec3I V3IX;
extern const Vec3I V3IY;
extern const Vec3I V3IZ;

inline int Dot(
	const Vec3I& v0,
	const Vec3I& v1)
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

inline Vec3I Cross(
	const Vec3I& v0,
	const Vec3I& v1)
{
	Vec3I retval;

	retval.x = v0.y * v1.z - v0.z * v1.y;
	retval.y = v0.z * v1.x - v0.x * v1.z;
	retval.z = v0.x * v1.y - v0.y * v1.y;

	return retval;
}

inline int SqrLen(
	const Vec3I& v0)
{
	return Dot(v0, v0);
}

/*
inline float Len(
	const Vec3I& v0)
{
	return Sqrt((float)Dot(v0, v0));
}
*/

/*
inline VecI Norm(
	const VecI& v0)
{
	float length = Len(v0);
	//XASSERT(length > EPSILON);

	return v0 / length;
}
*/

inline Vec3I Vec3ISet(int x, int y, int z)
{
	Vec3I retval;

	retval.x = x;
	retval.y = y;
	retval.z = z;

	return retval;
}

inline Vec3I V3I(int x, int y, int z)
{
	Vec3I retval;

	retval.x = x;
	retval.y = y;
	retval.z = z;

	return retval;
}

inline Vec3I MulEl3(
	const Vec3I &v0, 
	const Vec3I &v1)
{
	Vec3I retval;

	retval.x = v0.x * v1.x;
	retval.y = v0.y * v1.y;
	retval.z = v0.z * v1.z;

	return retval;
}

inline Vec3I ElDiv(
	const Vec3I &v0, 
	const Vec3I &v1)
{
	Vec3I retval;

	retval.x = v0.x / v1.x;
	retval.y = v0.y / v1.y;
	retval.z = v0.z / v1.z;

	return retval;
}

inline int SqrDist(const Vec3I& v1, const Vec3I& v2)
{
	Vec3I diff = v2-v1;
	return SqrLen(diff);
}
