#ifndef RENDERER_COLOR_H
#define RENDERER_COLOR_H

#include <Math/Math.h>

struct noLinearColor {
	float r, g, b, a;

	noLinearColor() {}
	noLinearColor(float _r, float _g, float _b, float _a = 1.0f) 
		:a(_a), r(_r), g(_g), b(_b) {}
	//noLinearColor(const noColor&c );
	//noColor ToRGBE() const;

	float& Component( int index) 
	{
		return (&r)[index];
	}
	const float& Component( int index) const
	{
		return (&r)[index];
	}

	noLinearColor operator+(const noLinearColor& c) const {
		return noLinearColor(
			this->r + c.r, 
			this->g + c.g, 
			this->b + c.b,
			this->a + c.a);
	}

	noLinearColor& operator+=(const noLinearColor& ColorB)
	{
		r += ColorB.r;
		g += ColorB.g;
		b += ColorB.b;
		a += ColorB.a;
		return *this;
	}

	noLinearColor operator-(const noLinearColor& ColorB) const
	{
		return noLinearColor(
			this->r - ColorB.r,
			this->g - ColorB.g,
			this->b - ColorB.b,
			this->a - ColorB.a
			);
	}
	noLinearColor& operator-=(const noLinearColor& ColorB)
	{
		r -= ColorB.r;
		g -= ColorB.g;
		b -= ColorB.b;
		a -= ColorB.a;
		return *this;
	}

	noLinearColor operator*(const noLinearColor& ColorB) const
	{
		return noLinearColor(
			this->r * ColorB.r,
			this->g * ColorB.g,
			this->b * ColorB.b,
			this->a * ColorB.a
			);
	}
	noLinearColor& operator*=(const noLinearColor& ColorB)
	{
		r *= ColorB.r;
		g *= ColorB.g;
		b *= ColorB.b;
		a *= ColorB.a;
		return *this;
	}

	noLinearColor operator*(float Scalar) const
	{
		return noLinearColor(
			this->r * Scalar,
			this->g * Scalar,
			this->b * Scalar,
			this->a * Scalar
			);
	}

	noLinearColor& operator*=(float Scalar)
	{
		r *= Scalar;
		g *= Scalar;
		b *= Scalar;
		a *= Scalar;
		return *this;
	}

	noLinearColor operator/(const noLinearColor& ColorB) const
	{
		return noLinearColor(
			this->r / ColorB.r,
			this->g / ColorB.g,
			this->b / ColorB.b,
			this->a / ColorB.a
			);
	}
	noLinearColor& operator/=(const noLinearColor& ColorB)
	{
		r /= ColorB.r;
		g /= ColorB.g;
		b /= ColorB.b;
		a /= ColorB.a;
		return *this;
	}

	noLinearColor operator/(float Scalar) const
	{
		const float	InvScalar = 1.0f / Scalar;
		return noLinearColor(
			this->r * InvScalar,
			this->g * InvScalar,
			this->b * InvScalar,
			this->a * InvScalar
			);
	}
	noLinearColor& operator/=(float Scalar)
	{
		const float	InvScalar = 1.0f / Scalar;
		r *= InvScalar;
		g *= InvScalar;
		b *= InvScalar;
		a *= InvScalar;
		return *this;
	}

	/** Comaprison operators */
	bool operator==(const noLinearColor& ColorB) const
	{
		return this->r == ColorB.r && this->g == ColorB.g && this->b == ColorB.b && this->a == ColorB.a;
	}
	bool operator!=(const noLinearColor& Other) const
	{
		return this->r != Other.r || this->g != Other.g || this->b != Other.b || this->a != Other.a;
	}

	// Error-tolerant comparison.
	bool Equals(const noLinearColor& ColorB, float Tolerance=1e-4) const
	{
		return noMath::Fabs(this->r - ColorB.r) < Tolerance && noMath::Fabs(this->g - ColorB.g) < Tolerance && noMath::Fabs(this->b - ColorB.b) < Tolerance && noMath::Fabs(this->a - ColorB.a) < Tolerance;
	}
};

inline noLinearColor operator*(FLOAT Scalar,const noLinearColor& Color)
{
	return Color.operator*( Scalar );
}

class noColor {
public:
	union { struct{ BYTE b,g,r,a; }; DWORD AlignmentDummy; };

	
	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}


	noColor() {}
	noColor( BYTE _r, BYTE _g, BYTE _b, BYTE _a = 255) 
		:a(_a), r(_r), g(_g), b(_b) {}

	explicit noColor( DWORD color) 
	{
		DWColor() = color;		
	}

	// Operators.
	bool operator==( const noColor &C ) const
	{
		return DWColor() == C.DWColor();
	}
	bool operator!=( const noColor& C ) const
	{
		return DWColor() != C.DWColor();
	}

};

inline DWORD GetTypeHash( const noColor& Color )
{
	return Color.DWColor();
}

#endif