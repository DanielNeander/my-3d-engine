/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef __IMAGE_H__
#define __IMAGE_H__

//#include "Vector.h"



class Pixel
{
public:
    /// The data.
    union {
            float    m_c[3];
        struct {
            float    r;
            float    g;
            float    b;
        };
    };

	//Pixel(unsigned char ir, unsigned char ig, unsigned char ib) { Set(ir, ig, ib); }
	Pixel( float fr, float fg, float fb ) { r = fr; g = fg; b = fb; }
    Pixel(){ r = 0; g = 0; b = 0; }
    void Set(unsigned char ir, unsigned char ig, unsigned char ib) { r = (float)ir; g = (float)ig; b = (float)ib; }
    void Set(float ir, float ig, float ib) { r = ir; g = ig; b = ib; }

	bool IsBlack() const { return (r == 0) && (g == 0) && (b == 0); }
    /// assign operator
    Pixel&   operator=(const Pixel & rightop);
    /// += operator
    Pixel&   operator+=(const Pixel & rightop);
    /// -= operator
    Pixel&   operator-=(const Pixel & rightop);
    /// *= operator
    Pixel&   operator*=(const Pixel & rightop);
    /// /= operator
    Pixel&   operator/=(const Pixel & rightop);
    /// scalar *= operator
    Pixel&   operator*=(float rightop);
    /// scalar /= operator
    Pixel&   operator/=(float rightop);

    /// RGB * f
    friend Pixel operator*( const Pixel& c, float f );
    /// f * RGB
    friend Pixel operator*( float f, const Pixel& c );
    /// RGB / f
    friend Pixel operator/( const Pixel& c, float f );
    // binary ops
    /// RGB1 * RGB2
    friend Pixel operator*( const Pixel& c1, const Pixel& c2 );
    /// RGB1 / RGB2
    friend Pixel operator/( const Pixel& c1, const Pixel& c2 );
    /// RGB1 + RGB2
    friend Pixel operator+( const Pixel& c1, const Pixel& c2 );
    /// RGB1 - RGB2
    friend Pixel operator-( const Pixel& c1, const Pixel& c2 );

};

inline Pixel operator*(const Pixel& c, float f)
{
    return Pixel(c.r*f, c.g*f, c.b*f);
}
inline Pixel operator*(float f, const Pixel& c)
{
    return Pixel(c.r*f, c.g*f, c.b*f);
}
inline Pixel operator/(const Pixel& c, float f)
{
    return Pixel(c.r/f, c.g/f, c.b/f);
}

inline Pixel& Pixel::operator=(const Pixel & rightop) {
    r = rightop.r;
    g = rightop.g;
    b = rightop.b;
    return *this;
}
inline Pixel& Pixel::operator+=(const Pixel & rightop) {
    *this = *this + rightop;
    return *this;
}

inline Pixel& Pixel::operator-=(const Pixel & rightop) {
    *this = *this - rightop;
    return *this;
}
inline Pixel& Pixel::operator*=(const Pixel & rightop) {
    *this = *this * rightop;
    return *this;
}
inline Pixel& Pixel::operator*=(float rightop) {
    *this = *this * rightop;
    return *this;
}
inline Pixel& Pixel::operator/=(float rightop) {
    *this = *this / rightop;
    return *this;
}

inline Pixel operator*(const Pixel& c1, const Pixel& c2)
{
    return Pixel(c1.r*c2.r, c1.g*c2.g, c1.b*c2.b);
}
inline Pixel operator/(const Pixel& c1, const Pixel& c2)
{
    return Pixel(c1.r/c2.r, c1.g/c2.g, c1.b/c2.b);
}
inline Pixel operator+(const Pixel& c1, const Pixel& c2)
{
    return Pixel(c1.r+c2.r, c1.g+c2.g, c1.b+c2.b);
}
inline Pixel operator-(const Pixel& c1, const Pixel& c2)
{
    return Pixel(c1.r-c2.r, c1.g-c2.g, c1.b-c2.b);
}

class ImagePRT
{
public:
	typedef struct _TAGRgb
    {
        unsigned char r, g, b;
	} Rgb;



    ImagePRT();
    virtual ~ImagePRT();

    void Resize(int width, int height);
    //noVec3 GetPixel(int x, int y) const;

	int GetPixel( Pixel* clr, int x, int y ) const {
		if (x >= 0 && x < m_iWidth && y < m_iHeight && y >= 0) {
			const Pixel& pix = m_pPixels[y*m_iWidth+x];
			*clr = pix;
			return 1;
		}
		*clr = Pixel (0,0,0);
		return 0;
	}

    void WritePPM(const char* pcFile); // write data to a ppm image file
    void WritePPM(const char *pcName, unsigned char *data, int width, int height);

    inline int Width() { return m_iWidth; }
    inline int Height() { return m_iHeight; }

private:
    Pixel* m_pPixels;
    int m_iWidth;
    int m_iHeight;
};

#endif
