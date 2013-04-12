#ifndef __CLIPMAP_H__
#define __CLIPMAP_H__

#include "JPEG_Preprocessor.h"

#define SPHERE_MERIDIAN_SLICES_NUM 128
#define SPHERE_PARALLEL_SLICES_NUM 128


#define SOURCE_FILES_NUM 1

// Nvidia D3D10 SDK Version
class Clipmaps : Singleton<Clipmaps>
{
public:
	Clipmaps(ID3D11Device* Dev, ID3D11DeviceContext* context);
	

	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	int ClipmapStackSize;
	int UpdateRegionSize;
	int RenderingTechnique;
	
	bool Init();

	
	void Reset();

	void LoadClipmaps();

	void SetTexurePrefix(const std::wstring& name) { TexnamePrefix = name; }

	ID3D11Texture2D* GetStackTexture()	const { return StackTexture; }
	void Process( int clipmapSize);
	void SetClipmapVars();
	void UpdateClipMap();
	void ReleaseTextures();


	ID3DX11Effect*	ClipmapEffect;	

//private:
	void LoadShaders();
	void CalculateClipmapParameters();
	HRESULT CreateClipmapTextures( ID3D11Device* pd3dDevice );
	void        InitStackTexture( ID3D11Device* pd3dDevice );
	void        UpdateStackTexture( ID3D11Device* pd3dDevice );
	void        UpdateMipPosition( int &position, int offset );
	D3DXVECTOR2			StackPosition;            // Virtual clipmap stack position (defined using normalized texture coordinates)
	int					StackDepth;                // Number of layers in a clipmap stack

	std::vector<int>    StackSizeList;
	std::vector<int>    UpdateRegionSizeList;

	ID3D11Texture2D*             PyramidTexture; // Texture which is used to store coarse mip levels
	ID3D11ShaderResourceView*    PyramidTextureSRV;
	ID3D11Texture2D*             PyramidTextureHM; // Height map for pyramid texture
	ID3D11ShaderResourceView*    PyramidTextureHMSRV;
	ID3D11Texture2D*             StackTexture; // Clipmap stack texture
	ID3D11ShaderResourceView*    StackTextureSRV;

	int SourceImageWidth, SourceImageHeight;
	int SourceImageMipsNum;

	int**		SourceImageMipsSize;
	int**		UpdatePositions;                           // Defines positions for each clipmap layer where new data should be placed

	int*       MipDataOffset;     // Offset in bytes for each mip level data stored in a temporary file

	std::wstring	TexnamePrefix;
	std::vector<std::wstring>	Texturenames;
	std::vector<std::wstring>	Hmfilenames;

	noVec3 EyePosition;

	WCHAR DstMediaPath[SOURCE_FILES_NUM][MAX_PATH];
	WCHAR DstMediaPathHM[SOURCE_FILES_NUM][MAX_PATH];
};

extern JPEG_Manager JPEManager;

// GPG7 Version
class Clipmap {
public:
		

	ID3DX11Effect*	ClipmapEffect;	

	idStr	SrcTexutreName;
	// Total storage for the raw image we'll be clipmapping.
	unsigned char *mRawImage;

	// Pointers to each mip level of the image.
	unsigned char *mRawMips[12];

	int csmClipMapSize;
	int csmClipMapShift; 
	int csmSourceImageSize;
	int csmSourceImageMipCount;

	// Simple integer-coordinate point.
	struct Point2I
	{
		Point2I()
		{
		}

		Point2I(int inX, int inY)
			: x(inX), y(inY)
		{
		}

		int x,y;

		Point2I  operator-(const Point2I &_rSub) const
		{
			return Point2I(x - _rSub.x, y - _rSub.y);
		}
	};

	// Simple integer-coordinate rectangle.
	struct RectI
	{
		RectI()
		{
		}

		RectI(Point2I p, Point2I e)
		{
			point = p;
			extent = e;
		}

		Point2I point, extent;

		bool isValidRect() const { return (extent.x > 0 && extent.y > 0); }

		bool intersect(const RectI &clipRect)
		{
			Point2I bottomL;
			bottomL.x = min(point.x + extent.x - 1, clipRect.point.x + clipRect.extent.x - 1);
			bottomL.y = min(point.y + extent.y - 1, clipRect.point.y + clipRect.extent.y - 1);

			point.x = max(point.x, clipRect.point.x);
			point.y = max(point.y, clipRect.point.y);

			extent.x = bottomL.x - point.x + 1;
			extent.y = bottomL.y - point.y + 1;

			return isValidRect();
		}

		bool overlaps(RectI R) const
		{
			return R.intersect (* this);
		}

		bool contains(const RectI& R) const
		{
			if (point.x <= R.point.x && point.y <= R.point.y)
				if (R.point.x + R.extent.x <= point.x + extent.x)
					if (R.point.y + R.extent.y <= point.y + extent.y)
						return true;
			return false;
		}

	};

	// Each layer of the clipmap stack is represented by one of these structures.
	struct ClipStackEntry
	{
		// The top-left coordinate in the virtualized image that this entry
		// currently stores. It's referred to as the toroidal offset because 
		// this modulo clipmapsize indicates where the seams in the toroidal
		// image buffer is.
		Point2I mToroidalOffset;

		/// The texture coordinate scale factor for this level.
		float mScale;

		/// Centerpoint of this layer in UV coordinates.
		float mCenterX, mCenterY;

		/// The actual texture resource this stack entry owns.
		class D3D11Texture2D *mTex;
		ID3D11ShaderResourceView*	mTexSRV;

		ClipStackEntry()
		{
			mToroidalOffset.x = mToroidalOffset.y = 0;
			mCenterX = mCenterY = 0;
			mScale = 1.f;
			mTex = NULL;
			mTexSRV = NULL;
		}
		~ClipStackEntry()
		{
			SAFE_RELEASE(mTexSRV);
			SAFE_DELETE(mTex);
		}
	};

	static const int csmLayerCount = 5;


	ClipStackEntry mLayer[csmLayerCount];

	Clipmap()
	{	
		mRawImage = NULL;
		ClipmapEffect = NULL;
	}

	void reset();

	void alloc(ID3D11Device *device);
	

	~Clipmap()
	{
		reset();
		if(mRawImage)
		{
			delete[] mRawImage;
			mRawImage = NULL;
		}
	}

	void bindTextures();
	void updateEffectParams();	
	void generateMip32Bit(unsigned char *srcLevel, int srcSize, unsigned char *outLevel);
	void fillSourceData();
	void clipAgainstGrid(const int gridSpacing, const RectI &rect, int *outCount, RectI *outBuffer);

	void calculateModuloDeltaBounds(const RectI &oldData, const RectI &newData, 
		RectI *outRects, int *outRectCount);

	int clamp(int val, int min, int max)
	{
		if(val < min) return min;
		else if(val > max) return max;
		else               return val;
	}

	void uploadToTexture(int level, RectI srcRegion, RectI dstRegion);
	void recenter(float centerX, float centerY);
	/// This is called when we want to completely fill every layer; for instance
	/// for the first frame or as part of a device reset.
	void fillWithTextureData();
	bool Init();
};

extern Clipmap* gClipMap2;

#endif