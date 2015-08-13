// Vec2I.h
#pragma once

struct Vec2I
{
	int x;
	int y;
};

inline bool operator==(const Vec2I& v1, const Vec2I& v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

inline Vec2I V2I(int x, int y)
{
	Vec2I ret;
	ret.x = x;
	ret.y = y;
	return ret;
}

inline Vec2I operator-(const Vec2I &p1, const Vec2I& p2)
{
	return V2I(p1.x - p2.x, p1.y - p2.y);
}

inline Vec2I operator+(const Vec2I &p1, const Vec2I& p2)
{
	return V2I(p2.x + p1.x, p2.y + p1.y);
}

inline int SqrDist(const Vec2I& p1, const Vec2I& p2)
{
	Vec2I dist = p2 - p1;
	return (dist.x * dist.x + dist.y + dist.y);
}

/*
extern const Vec2I V2I0;
extern const Vec2I V2I1;
extern const Vec2I V2IX;
extern const Vec2I V2IY;

TGINLINE Vec2I Vec2ISet(int x, int y);

//struct V2I_ : Vec2I
//{
//	V2I_(int _x, int _y) {
//		x = _x;
//		y = _y;
//	}
//};
//
//#define V2I(x, y) V2I_(x, y)

TGINLINE Vec2I V2I(int x, int y);


TGINLINE Vec2I Vec2RoundToInt(const Vec2 &v);
TGINLINE Vec2I Vec2FloorToInt(const Vec2 &v);
TGINLINE Vec2I Vec2CeilToInt(const Vec2 &v);

TGINLINE bool operator ==(const Vec2I& v1, const Vec2I& v2);
TGINLINE bool operator !=(const Vec2I& v1, const Vec2I& v2);
TGINLINE void operator +=(Vec2I& v1, const Vec2I& v2);
TGINLINE Vec2I operator +(const Vec2I& v1, const Vec2I& v2);
TGINLINE void operator -=(Vec2I& v1, const Vec2I& v2);
TGINLINE Vec2I operator -(const Vec2I& v1, const Vec2I& v2);
TGINLINE void operator *=(Vec2I& v, int f);
TGINLINE Vec2I operator *(const Vec2I& v, int f);
TGINLINE Vec2I operator*(int f, const Vec2I& v);
//void operator /=(Vec2I& v, float f); 
//Vec2I operator /(const Vec2I& v, int f);
TGINLINE Vec2I operator -(const Vec2I& v);
TGINLINE Vec2I MulEl2(const Vec2I& v0, const Vec2I& v1);

TGINLINE int   Dot(const Vec2I& v0,const Vec2I& v1);
TGINLINE int   SqrLen(const Vec2I& v0);
TGINLINE float Len(const Vec2I& v0);
TGINLINE int   SqrDist(const Vec2I& v0, const Vec2I& v1);
TGINLINE float Dist(const Vec2I& v0, const Vec2I& v1);
TGINLINE Vec2I Perp(const Vec2I& v);
TGINLINE bool  RangeContains(const Vec2I &r, float p);
TGINLINE void  RangeInflate(Vec2I &v, float p);
TGINLINE Vec2I RangeIntersect(const Vec2I &r0, const Vec2I &r1);
TGINLINE Vec2I RangeUnion(const Vec2I &r0, const Vec2I &r1);
TGINLINE Vec2I LerpInt(const Vec2I& v1, const Vec2I& v2, float t);
*/
