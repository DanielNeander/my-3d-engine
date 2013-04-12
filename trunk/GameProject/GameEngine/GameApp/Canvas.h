#pragma once 

class FCanvas
{
public:	
	/** 
	 * Enum that describes what type of element we are currently batching.
	 */
	enum EElementType
	{
		ET_Line,
		ET_Triangle,
		ET_MAX
	};

	FCanvas(class FRenderTarget* InRenderTarget);
	~FCanvas();
	/** 
	* Sends a message to the rendering thread to draw the batched elements. 
	*/
	void Flush();

		/**
	 * Pushes a transform onto the canvas's transform stack, multiplying it with the current top of the stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushRelativeTransform(const FMatrix& Transform);

	/**
	 * Pushes a transform onto the canvas's transform stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushAbsoluteTransform(const FMatrix& Transform);

	/**
	 * Removes the top transform from the canvas's transform stack.
	 */
	void PopTransform();

	/**
	* Replace the base (ie. TransformStack(0)) transform for the canvas with the given matrix
	*
	* @param Transform - The transform to use for the base
	*/
	void SetBaseTransform(const FMatrix& Transform);

	/**
	* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform2D(UINT ViewSizeX, UINT ViewSizeY);

	/**
	* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform3D(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);
	
	/**
	* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @return Matrix for canvas view orientation
	*/
	static FMatrix CalcViewMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV);
	
	/**
	* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcProjectionMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);

	/**
	* Get the current top-most transform entry without the canvas projection
	* @return matrix from transform stack. 
	*/
	FMatrix GetTransform() const 
	{ 
		return TransformStack.Top().GetMatrix() * TransformStack(0).GetMatrix().Inverse(); 
	}

	/** 
	* Get the bottom-most element of the transform stack. 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetBottomTransform() const 
	{ 
		return TransformStack(0).GetMatrix(); 
	}

	/**
	* Get the current top-most transform entry 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetFullTransform() const 
	{ 
		return TransformStack.Top().GetMatrix(); 
	}

	/**
	* Toggles current depth testing state for the canvas. All batches
	* will render with depth testing against the depth buffer if enabled.
	*
	* @param bEnabled - if TRUE then depth testing is enabled
	*/
	void SetDepthTestingEnabled(UBOOL bEnabled);

	/** 
	 * Set the current masked region on the canvas
	 * All rendering from this point on will be masked to this region.
	 * The region being masked uses the current canvas transform
	 *
	 * @param X - x offset in canvas coords
	 * @param Y - y offset in canvas coords
	 * @param SizeX - x size in canvas coords
	 * @param SizeY - y size in canvas coords
	 */
	void PushMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY );

	/**
	 * Replace the top element of the masking region stack with a new region
	 */
	void ReplaceMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY );

	/**
	 * Remove the current masking region; if other masking regions were previously pushed onto the stack,
	 * the next one down will be activated.
	 */
	void PopMaskRegion();

	/**
	 * Sets the render target which will be used for subsequent canvas primitives.
	 */
	void SetRenderTarget(FRenderTarget* NewRenderTarget);	

	/**
	* Get the current render target for the canvas
	*/	
	FORCEINLINE FRenderTarget* GetRenderTarget() const 
	{ 
		return RenderTarget; 
	}

	/**
	* Marks render target as dirty so that it will be resolved to texture
	*/
	void SetRenderTargetDirty(UBOOL bDirty) 
	{ 
		bRenderTargetDirty = bDirty; 
	}

	/**
	* Push sort key onto the stack. Rendering is done with the current sort key stack entry.
	*
	* @param InSortKey - key value to push onto the stack
	*/
	void PushDepthSortKey(INT InSortKey)
	{
		DepthSortKeyStack.Push(InSortKey);
	};
	/**
	* Pop sort key off of the stack.
	*
	* @return top entry of the sort key stack
	*/
	INT PopDepthSortKey()
	{
		INT Result = 0;
		if( DepthSortKeyStack.Num() > 0 )
		{
			Result = DepthSortKeyStack.Pop();
		}
		else
		{
			// should always have one entry
			PushDepthSortKey(0);
		}
		return Result;		
	};
	/**
	* Return top sort key of the stack.
	*
	* @return top entry of the sort key stack
	*/
	INT TopDepthSortKey()
	{
		assert(DepthSortKeyStack.Num() > 0);
		return DepthSortKeyStack.Top();
	}

public:
	FLOAT AlphaModulate;

	/** Entry for the transform stack which stores a matrix and its CRC for faster comparisons */
	class FTransformEntry
	{
	public:
		FTransformEntry(const FMatrix& InMatrix)
			:	Matrix(InMatrix)
		{			
			MatrixCRC = appMemCrc(&Matrix,sizeof(FMatrix));
		}
		FORCEINLINE void SetMatrix(const FMatrix& InMatrix)
		{
			Matrix = InMatrix;
			MatrixCRC = appMemCrc(&Matrix,sizeof(FMatrix));
		}
		FORCEINLINE const FMatrix& GetMatrix() const
		{
			return Matrix;
		}
		FORCEINLINE DWORD GetMatrixCRC() const
		{
			return MatrixCRC;
		}
	private:
		FMatrix Matrix;
		DWORD MatrixCRC;
	};

	private:
		/** Current render target used by the canvas */
		FRenderTarget* RenderTarget;
		/** TRUE to enable depth testing. Useful when using a canvas scene for primitives */
		UBOOL bEnableDepthTest;
		/** TRUE if the render target has been rendered to since last calling SetRenderTarget() */
		UBOOL bRenderTargetDirty;	


		/** Stack of SortKeys. All rendering is done using the top most sort key */
	TArray<INT> DepthSortKeyStack;	
	/** Stack of matrices. Bottom most entry is the canvas projection */
	TArray<FTransformEntry> TransformStack;	
	
	/** 
	* Region on the canvas that should be masked
	*/
	struct FMaskRegion
	{
		FMaskRegion(FLOAT InX=-1,FLOAT InY=-1,FLOAT InSizeX=-1,FLOAT InSizeY=-1, const FMatrix& InTransform=mat4_identity) 
			: X(InX), Y(InY), SizeX(InSizeX), SizeY(InSizeY), Transform(InTransform) 
		{}
		FORCEINLINE UBOOL IsEqual(const FMaskRegion& R) const		
		{ 
			return(	noMath::Fabs(X-R.X) < KINDA_SMALL_NUMBER && 
					noMath::Fabs(Y-R.Y) < KINDA_SMALL_NUMBER && 
					noMath::Fabs(SizeX-R.SizeX) < KINDA_SMALL_NUMBER && 
					noMath::Fabs(SizeY-R.SizeY) < KINDA_SMALL_NUMBER );
		}
		UBOOL IsValid() const 
		{ 
			return X >= -DELTA && Y >= -DELTA && SizeX >= -DELTA && SizeY >= -DELTA; 
		}
		FORCEINLINE UBOOL IsZero(FLOAT Tolerance=DELTA) const
		{
			//@todo - do we need to check tranform?
			return noMath::Fabs(X) < noMath::Fabs(Tolerance)
				&& noMath::Fabs(Y) < noMath::Fabs(Tolerance)
				&& noMath::Fabs(SizeX) < Abs(Tolerance)
				&& Abs(SizeY) < Abs(Tolerance);
		}
		FLOAT X,Y,SizeX,SizeY;
		FMatrix Transform;
	};

	/**
	 * Stack of mask regions - top of stack (last element) is current canvas mask
	 */
	TArray<FMaskRegion> MaskRegionStack;

public:
	/**
	 * Get the top-most canvas masking region from the stack.
	 */
	FMaskRegion GetCurrentMaskRegion() const;
private:


	/** 
	* Contains all of the batched elements that need to be rendered at a certain depth sort key
	*/
	class FCanvasSortElement
	{
	public:
		/** 
		* Init constructor 
		*/
		FCanvasSortElement(INT InDepthSortKey=0)
			:	DepthSortKey(InDepthSortKey)
		{}

		/** 
		* Equality is based on sort key 
		*
		* @param Other - instance to compare against
		* @return TRUE if equal
		*/
		UBOOL operator==(const FCanvasSortElement& Other) const
		{
			return DepthSortKey == Other.DepthSortKey;
		}

		/** sort key for this set of render batch elements */
		INT DepthSortKey;
		/** list of batches that should be rendered at this sort key level */
		TArray<class FCanvasBaseRenderItem*> RenderBatchArray;
	};

	/** FCanvasSortElement compare class */
	IMPLEMENT_COMPARE_CONSTREF(FCanvasSortElement,UnCanvas,{ return A.DepthSortKey <= B.DepthSortKey ? 1 : -1;	})	
		/** Batched canvas elements to be sorted for rendering. Sort order is back-to-front */
		TArray<FCanvasSortElement> SortedElements;
	/** Map from sortkey to array index of SortedElements for faster lookup of existing entries */
	std::map<int, int> SortedElementLookupMap;

	/**
	* Get the sort element for the given sort key. Allocates a new entry if one does not exist
	*
	* @param DepthSortKey - the key used to find the sort element entry
	* @return sort element entry
	*/
	FCanvasSortElement& GetSortElement(INT DepthSortKey);

	/**
	* Setup the current masked region during flush
	*/
	void FlushSetMaskRegion();
	/**
	* Clear masked region during flush
	*/
	void FlushResetMaskRegion();
};

extern void DrawTriangle2D(
	FCanvas* Canvas,
	const FIntPoint& Position0,
	const FIntPoint& TexCoord0,
	const FIntPoint& Position1,
	const FIntPoint& TexCoord1,
	const FIntPoint& Position2,
	const FIntPoint& TexCoord2,
	const FLinearColor& Color,
	UBOOL AlphaBlend = 1
	);
extern void DrawLine2D(FCanvas* Canvas, const FIntPoint& StartPos, const FIntPoint& EndPos, const FLinearColor& Color );

extern void DrawBox2D(FCanvas* Canvas,const FIntPoint& StartPos,const FIntPoint& EndPos,const FLinearColor& Color);
extern void DrawTile( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY, FLOAT U, FLOAT V, FLOAT SizeU, FLOAT SizeV, const FLinearColor& Color, UBOOL AlphaBlend );