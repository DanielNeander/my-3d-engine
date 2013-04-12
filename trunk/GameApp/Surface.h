#pragma once 

#ifndef __INTEL_COMPILER
//#define restrict
//inline float _mm_cvtss_f32(__m128 v) { float r; _mm_store_ss(&r, v); return r; }
#endif

class Surface
{
public:
	// constructor / destructors
	Surface( int a_Width, int a_Height, unsigned long* a_Buffer, int a_Pitch );
	Surface( int a_Width, int a_Height );
	~Surface();

	// member data access
	unsigned long* GetBuffer() { return m_Buffer; }
	void SetBuffer( unsigned long* a_Buffer ) { m_Buffer = a_Buffer; }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
	int GetPitch() { return m_Pitch; }
	void SetPitch( int a_Pitch ) { m_Pitch = a_Pitch; }

	// Special operations
	void InitCharset();
	void SetChar( int c, char* c1, char* c2, char* c3, char* c4, char* c5 );
	void Print( char* a_String, int x1, int y1, unsigned long color );
	void Clear( unsigned long a_Color );
	void Line( float x1, float y1, float x2, float y2, unsigned long color );
	void Plot( int x, int y, unsigned long color );
	void CopyTo( Surface* a_Dst, int a_X, int a_Y );

private:
	// Attributes
	unsigned long* m_Buffer;	
	int m_Width, m_Height, m_Pitch;	

	// Static attributes for the buildin font
	char s_Font[51][5][5];	
	int s_Transl[256];		
};