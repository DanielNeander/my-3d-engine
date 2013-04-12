#pragma once 

struct surface {
	float vertices[4][3];
	float matrix[9];

	float s_dist, t_dist;	
};
unsigned int generate_lightmap(struct surface *surf);

class LightMap 
{
public:

	struct SortCoordsByDistance
	{
		bool operator()(std::pair<int, int> &left, std::pair<int, int> &right)
		{
			return (left.first*left.first + left.second*left.second) < 
				(right.first*right.first + right.second*right.second);
		}
	};

protected:
	/*void LightTriangle(const noVec3 &P1, const noVec3 &P2, const noVec3 &P3,
		const noVec3 &N1, const noVec3 &N2, const noVec3 &N3,
		const Vector2 &T1, const Vector2 &T2, const Vector2 &T3);
	uint8 GetLightIntensity(const noVec3 &Position, const noVec3 &Normal);
	bool CalculateLightMap();
	void AssignMaterial();
	void CreateTexture();
	void FillInvalidPixels();
	static void BuildSearchPattern();

	/// Convert between texture coordinates given as reals and pixel coordinates given as integers
	inline int GetPixelCoordinate(Real TextureCoord);
	inline Real GetTextureCoordinate(int iPixelCoord);

	/// Calculate coordinates of P in terms of P1, P2 and P3
	/// P = x*P1 + y*P2 + z*P3
	/// If any of P.x, P.y or P.z are negative then P is outside of the triangle
	noVec3 GetBarycentricCoordinates(const Vector2 &P1, const Vector2 &P2, const Vector2 &P3, const Vector2 &P);
	/// Get the surface area of a triangle
	Real GetTriangleArea(const noVec3 &P1, const noVec3 &P2, const noVec3 &P3);

	TexturePtr m_Texture;
	MaterialPtr m_Material;
	SubEntity* m_pSubEntity;
	shared_ptr<cimg_library::CImg<unsigned char> > m_LightMap;
	String m_LightMapName;
	int m_iTexSize;
	int m_iCoordSet;
	static int m_iLightMapCounter;
	Real m_PixelsPerUnit;
	static vector<pair<int, int> > m_SearchPattern;
	bool m_bDebugLightmaps;*/
	
};