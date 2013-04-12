#pragma once 

class Point2I
{
	//-------------------------------------- Public static constants
public:
	const static Point2I One;
	const static Point2I Zero;

	//-------------------------------------- Public data
public:
	S32 x;   ///< X position
	S32 y;   ///< Y position

	//-------------------------------------- Public interface
public:
	Point2I();                               ///< Create an uninitialized point.
	Point2I(const Point2I&);                 ///< Copy constructor
	Point2I(S32 in_x, S32 in_y);             ///< Create point from two co-ordinates.

	//-------------------------------------- Non-math mutators and misc functions
	void set(S32 in_x, S32 in_y);            ///< Set (x,y) position
	void setMin(const Point2I&);             ///< Store lesser co-ordinates from parameter in this point.
	void setMax(const Point2I&);             ///< Store greater co-ordinates from parameter in this point.

	//-------------------------------------- Math mutators
	void neg();                              ///< Invert sign of point's co-ordinates.
	void convolve(const Point2I&);           ///< Convolve this point by parameter.

	//-------------------------------------- Queries
	bool isZero() const;                     ///< Is this point at the origin? (No epsilon used)
	F32  len() const;                        ///< Get the length of the point
	S32  lenSquared() const;                 ///< Get the length-squared of the point

	//-------------------------------------- Overloaded operators
public:
	operator S32*() { return &x; }
	operator const S32*() const { return &x; }

	// Comparison operators
	bool operator==(const Point2I&) const;
	bool operator!=(const Point2I&) const;

	// Arithmetic w/ other points
	Point2I  operator+(const Point2I&) const;
	Point2I  operator-(const Point2I&) const;
	Point2I& operator+=(const Point2I&);
	Point2I& operator-=(const Point2I&);

	// Arithmetic w/ scalars
	Point2I  operator*(S32) const;
	Point2I& operator*=(S32);
	Point2I  operator/(S32) const;
	Point2I& operator/=(S32);

	// Unary operators
	Point2I operator-() const;
};

inline Point2I::Point2I()
{

}

inline Point2I::Point2I( const Point2I& _copy)
	: x(_copy.x), y(_copy.y)
{

}

inline Point2I::Point2I( S32 in_x, S32 in_y )
	: x(_x), y(_y)

{

}

void inline Point2I::set( S32 in_x, S32 in_y )
{
	x = _x;
	y = _y;
}
}

void inline Point2I::setMin( const Point2I& _test)
{
	x = (_test.x < x) ? _test.x : x;
	y = (_test.y < y) ? _test.y : y;
}

void inline Point2I::setMax( const Point2I& _test)
{
	x = (_test.x > x) ? _test.x : x;
	y = (_test.y > y) ? _test.y : y;
}

void inline Point2I::neg()
{
	x = -x;
	y = -y;
}

void inline Point2I::convolve( const Point2I& )
{
	x *= c.x;
	y *= c.y;
}

bool inline Point2I::isZero() const
{
	return ((x == 0) && (y == 0));

}

F32 inline Point2I::len() const
{
	return mSqrt(F32(x*x + y*y));

}

S32 inline Point2I::lenSquared() const
{
	return x*x + y*y;

}

bool inline Point2I::operator==( const Point2I& ) const
{
	return ((x == _test.x) && (y == _test.y));
}

Point2I inline Point2I::operator-( const Point2I& _rSub) const
{
	return Point2I(x - _rSub.x, y - _rSub.y);

}

Point2I& inline Point2I::operator-=( const Point2I& _test)
{
	x -= _rSub.x;
	y -= _rSub.y;

	return *this;
}

Point2I inline Point2I::operator*( S32 mul) const
{
	return Point2I(x * mul, y * mul);
}

Point2I& inline Point2I::operator*=( S32 mul)
{
	x *= mul;
	y *= mul;

	return *this;
}

Point2I& inline Point2I::operator+=( const Point2I& _add)
{
	x += _add.x;
	y += _add.y;

	return *this;
}

Point2I inline Point2I::operator+( const Point2I& _add) const
{
	return Point2I(x + _add.x, y + _add.y);
}

bool inline Point2I::operator!=( const Point2I& _test ) const
{
	return (operator==(_test) == false);

}

Point2I inline Point2I::operator/( S32 div) const
{
	return Point2I(x/div, y/div);
}

Point2I& inline Point2I::operator/=( S32 div)
{
	x /= div;
	y /= div;

	return *this;
}

Point2I inline Point2I::operator-() const
{
	return Point2I(-x, -y);

}
