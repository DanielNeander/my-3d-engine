#pragma once 


class TerrainFile;
class TerrainBlock;


TerrainBlock* getTerrainUnderWorldPoint(const noVec3 & wPos);

struct GridSquare
{
	U16 minHeight;
	U16 maxHeight;
	U16 heightDeviance;
	U16 flags;
	enum 
	{
		Split45 = 1,
		Empty = 2,
		HasEmpty = 4,
		MaterialShift = 3,
		MaterialStart = 8,
		Material0 = 8,
		Material1 = 16,
		Material2 = 32,
		Material3 = 64,
	};
};


struct GridChunk
{
	U16 heightDeviance[3]; // levels 0-1, 1-2, 2
	U16 emptyFlags;
};


class TerrainBlock : public SceneNode
{
	typedef SceneNode Parent;

public:
	struct Material 
	{
		enum Flags 
		{
			Plain          = 0,
			Rotate         = 1,
			FlipX          = 2,
			FlipXRotate    = 3,
			FlipY          = 4,
			FlipYRotate    = 5,
			FlipXY         = 6,
			FlipXYRotate   = 7,
			RotateMask     = 7,
			Empty          = 8,
			Modified       = BIT(7),

			// must not clobber TerrainFile::MATERIAL_GROUP_MASK bits!
			PersistMask       = BIT(7)
		};
		U8 flags;
		U8 index;
	};

	enum 
	{
		BlockSize = 256,
		BlockShift = 8,
		LightmapSize = 1024,
		LightmapShift = 10,
		ChunkSquareWidth = 64,
		ChunkSize = 4,
		ChunkDownShift = 2,
		ChunkShift = BlockShift - ChunkDownShift,
		BlockSquareWidth = 256,
		SquareMaxPoints = 1024,
		BlockMask = 255,
		GridMapSize = 0x15555,
		FlagMapWidth = 128, ///< Flags that map is for 2x2 squares.
		FlagMapMask = 127,
		MaxMipLevel = 6,
		NumBaseTextures = 16,
		MaterialGroups = 8,
		MaxEmptyRunPairs = 100
	};

	enum UpdateMaskBits
	{
		InitMask           = BIT(1),
		VisibilityMask     = BIT(2),
		TerrainChangedMask = BIT(3),
		EmptyMask          = BIT(4),
		MoveMask           = BIT(5),
	};

	SpeedTree::CTextureDirectX10	baseTextures[NumBaseTextures];
	SpeedTree::CTextureDirectX10	mBaseMaterials[MaterialGroups];
	SpeedTree::CTextureDirectX10	mAlphaMaterials[MaterialGroups];


	U8 *mBaseMaterialMap;	
	Material *materialMap;


	std::string *mMaterialFileName; ///< Array from the file.

	// fixed point height values
	U16 *heightMap;
	U16 *flagMap;
	//FileName mDetailTextureName;
	//GFXTexHandle mDetailTextureHandle;

	std::string mTerrFileName;
	std::vector<S32> mEmptySquareRuns;
	bool mHasRendered;

	F32 mDetailDistance;
	F32 mDetailBrightness;
	F32 mDetailScale;
		
	S32 mVertexBuffer;

	enum 
	{
		LightmapUpdate    = BIT(0),
		HeightmapUpdate   = BIT(1),
		OpacityUpdate     = BIT(2),
		EmptyUpdate       = BIT(3)
	};

	static Signal<void(U32,TerrainBlock*,const Point2I& ,const Point2I&)> smUpdateSignal;


	void updateOpacity();
	void addOpacitySources();

private:
	TerrainFile mFile;

	GridSquare *gridMap[BlockShift+1];
	GridChunk *mChunkMap;
	U32   mCRC;
	//BaseMatInstance* mTerrainMaterial;

	public:
		TerrainBlock();
		~TerrainBlock();
		void buildChunkDeviance(S32 x, S32 y);
		void buildGridMap();
		U32 getCRC() const { return(mCRC); }
		const TerrainFile& getFile() const { return mFile; };

		bool onAdd();
		void onRemove();

		void refreshMaterialLists();
		void onEditorEnable();
		void onEditorDisable();

		void rebuildEmptyFlags();
		bool unpackEmptySquares();
		void packEmptySquares();

		void inspectPostApply();

		Material *getMaterial(U32 x, U32 y);
		BaseMatInstance* getMaterialInst( U32 x, U32 y );
		GridSquare *findSquare(U32 level, const Point2I &pos);
		GridSquare *findSquare(U32 level, S32 x, S32 y);
		GridChunk *findChunk(const Point2I &pos);

		void calcBounds();

		void setHeight(const Point2I & pos, F32 height);
		void updateGrid(const Point2I &minPt, const Point2I &maxPt);
		void updateGridMaterials(const Point2I &minPt, const Point2I &maxPt);

		U16 getHeight(U32 x, U32 y) const;
		Point2I getGridPos( const Point3F& worldPos );

		U16 *getHeightAddress(U32 x, U32 y) 
		{
			return &heightMap[(x & BlockMask) + ((y & BlockMask) << BlockShift)]; 
		}

		void setBaseMaterial(U32 x, U32 y, U8 matGroup);

		U8 *getMaterialAlphaMap(U32 matIndex);
		U8* getBaseMaterialAddress(U32 x, U32 y);
		U8  getBaseMaterial(U32 x, U32 y);
		U16 *getFlagMapPtr(S32 x, S32 y);

		void getMaterialAlpha(const Point2I &pos, U8 alphas[MaterialGroups]) const;
		void setMaterialAlpha(const Point2I &pos, const U8 alphas[MaterialGroups]);

		// a more useful getHeight for the public...
		bool getHeight(const Point2F & pos, F32 * height);
		void getMinMaxHeight( F32* minHeight, F32* maxHeight ) const ;
		bool getNormal(const Point2F & pos, Point3F * normal, bool normalize = true);
		bool getNormalAndHeight(const Point2F & pos, Point3F * normal, F32 * height, bool normalize = true);

		// only the editor currently uses this method - should always be using a ray to collide with
		bool collideBox(const Point3F &start, const Point3F &end, RayInfo* info)
		{
			return(castRay(start,end,info));
		}

		S32 getMaterialAlphaIndex(const String &materialName);

private:
	S32 mSquareSize;
	S32 mTexelsPerMeter;
	S32 mClipMapSizeLog2; 
	bool mTile;

public:
	void setFile(Resource<TerrainFile> file);
	void createNewTerrainFile(StringTableEntry filename, U16 *hmap = NULL, Vector<StringTableEntry> *materials = NULL);
	bool save(const char* filename);

	static void flushCache();
	//void relight(const ColorF &lightColor, const ColorF &ambient, const Point3F &lightDir);

	S32 getSquareSize() const;
	void setSquareSize(S32 squareSize)
	{
		mSquareSize = squareSize;
	}
	bool isTiling() const;
	void setTiling(bool tiling)
	{
		mTile = tiling;
	}

	void refillClipmap();

	// collision info
private:
	S32 mHeightMin;	
	S32 mHeightMax;


	public:
		bool buildMaterialMap();
		void buildMipMap();

		void setBaseMaterials(S32 argc, const char *argv[]);
		bool loadBaseMaterials();
		bool initMMXBlender();

		// This method allows one to set an override material, this will cause the terrain to render itself
		// with just the overrideMaterial.  Call with NULL to restore normal terrain rendering.
		void setOverrideMaterial(BaseMatInstance* overrideMaterial);

		// Property accessors
		const FileName & getDetailTexture() const { return mDetailTextureName; }
		void setDetailTexture(const FileName &newValue) { mDetailTextureName = newValue; }

		const FileName &  getTerrainFile() const { return mTerrFileName; }
		void setTerrainFile(const FileName &newValue);

		String getMaterial(S32 idx) { if(mMaterialFileName != NULL && idx > -1 && idx < MaterialGroups) return mMaterialFileName[idx]; return String(""); }
		void setMaterial(S32 idx, const String &newValue);
		S32 getMaterialCount() const { return MaterialGroups; }

		void postLight(Vector<TerrainBlock *> &terrBlocks) {};


		private:
			bool        mCollideEmpty;

	public:
		static void initPersistFields();
		U32 packUpdate   (NetConnection *conn, U32 mask, BitStream *stream);
		void unpackUpdate(NetConnection *conn,           BitStream *stream);
};

// 11.5 fixed point - gives us a height range from 0->2048 in 1/32 inc

inline F32 fixedToFloat(U16 val)
{
	return F32(val) * 0.03125f;
}

inline U16 floatToFixed(F32 val)
{
	return U16(val * 32.0);
}
