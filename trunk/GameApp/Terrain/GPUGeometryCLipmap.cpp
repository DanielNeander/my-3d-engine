#include "stdafx.h"
#include "GPUGeom"


void GPUGeometryClipmap::Destroy()
{
	for (int l = 0; l < L; l++)
	{
		SAFE_RELEASE( Levels[l].ElevationMap );
		SAFE_RELEASE( Levels[l].NormalMap );
	}

	delete[] Levels;

	SAFE_RELEASE( PermTexture );
	SAFE_RELEASE( GradTexture );

	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pInstanceBuffer );
	SAFE_RELEASE( m_pIndexBufferMxM );
	SAFE_RELEASE( m_pIndexBufferMx3 );
	SAFE_RELEASE( m_pIndexBufferLShape[0] );
	SAFE_RELEASE( m_pIndexBufferLShape[1] );
	SAFE_RELEASE( m_pIndexBufferLShape[2] );
	SAFE_RELEASE( m_pIndexBufferLShape[3] );
	SAFE_RELEASE( m_pIndexBufferOuterDegenerateTriangles );
}


void GPUGeometryClipmap::Update(int Level, short Dx, short Dz)
{
	if ( Dx != 0 || Dz != 0 )
	{
		// Do update of levels that are rendered
		// for levels that are not rendered it only update coordinates

		// Update Vertex Translation
		Levels[Level].VecScale.z += Dx * Levels[Level].VecShift;
		Levels[Level].VecScale.w += Dz * Levels[Level].VecShift;

		// Update Texture Translation
		Levels[Level].TexScale.x += Dx * TexClipShift;
		Levels[Level].TexScale.y += Dz * TexClipShift;

		// Update L-Shape Indicator
		Levels[Level].Hpos = Levels[Level].Hpos + Dx;
		Levels[Level].Vpos = Levels[Level].Vpos + Dz;

		// Update Coarser Levels
		if ( Levels[Level].Hpos < 0 || Levels[Level].Hpos > 1 ||
			Levels[Level].Vpos < 0 || Levels[Level].Vpos > 1 )
		{
			short Mx = 0, My = 0;
			short TempDx = 0, TempDy = 0;

			if ( Dx < 0 ) Mx--;
			if ( Dz < 0 ) My--;

			TempDx = (Levels[Level].Hpos + Mx) / 2;
			TempDy = (Levels[Level].Vpos + My) / 2;

			if ( Level < L-1 ) Update( Level + 1, TempDx, TempDy );
		}
		else
		{
			// Check if coarser levels that were not rendered last frame ( and are now visible ) 
			// need to be partially or fully update 
			if ( Level < L-1 ) Update( Level + 1, 0, 0 );
		}

		// Update L-Shape
		Levels[Level].Hpos = (short)abs( Levels[Level].Hpos ) % 2 ;
		Levels[Level].Vpos = (short)abs( Levels[Level].Vpos ) % 2 ;

		// Update Texture Offset
		Dx *= 2;
		Dz *= 2;

		int Tpx = Levels[Level].TexTorusOrigin.x + Dx;
		int Tpy = Levels[Level].TexTorusOrigin.y + Dz;

		Levels[Level].TexTorusOrigin.x = Levels[Level].TexTorusOrigin.x + Dx;
		Levels[Level].TexTorusOrigin.y = Levels[Level].TexTorusOrigin.y + Dz;

		// Special case when short > 32767 or < -32767
		if ( Tpx > Levels[Level].TexTorusOrigin.x || Tpx < Levels[Level].TexTorusOrigin.x ||
			Tpy > Levels[Level].TexTorusOrigin.y || Tpy < Levels[Level].TexTorusOrigin.y) 
		{
			// /!\ /!\ TexTorusOrigin is a short, if displacement without update is > 32767 or < -32767, coordinates
			// will be wrong and data 's level will be wrong too. This "if" correct this.
			// i just added this feature, don't know if it's 100% safe yet but seems to work.
			if ( Dx > 0 ) {
				while ( Tpx >= TexClipSize ) { Tpx = Tpx - TexClipSize;	}
			} else {
				while ( Tpx <= 0 ) { Tpx = Tpx + TexClipSize; }
			}

			if ( Dz > 0 ) {
				while ( Tpy >= TexClipSize ) { Tpy = Tpy - TexClipSize; }
			} else {
				while ( Tpy <= 0 ) { Tpy = Tpy + TexClipSize; }
			}

			Levels[Level].TexTorusOrigin.x = (short)Tpx;
			Levels[Level].TexTorusOrigin.y = (short)Tpy;

			Levels[Level].ShortC = true;
		}

		// Update only clipmap levels that are rendered
		if ( Level >= MinLevel ) 
		{
			// Displacement when level was not visible + current displacement (level is visible)
			short Dx2 = (Levels[Level].TexTorusOrigin.x - Levels[Level].LastTexTorusOrigin.x);
			short Dz2 = (Levels[Level].TexTorusOrigin.y - Levels[Level].LastTexTorusOrigin.y);

			UpdateClipmap(Level, Dx2, Dz2); 
		}
	}
	else
	{
		if ( Level < L-1 ) Update( Level + 1, 0, 0 );

		if ( Level >= MinLevel ) 
		{
			// Displacement when level was not visible only (level is now visible)
			short Dx2 = (Levels[Level].TexTorusOrigin.x - Levels[Level].LastTexTorusOrigin.x);
			short Dz2 = (Levels[Level].TexTorusOrigin.y - Levels[Level].LastTexTorusOrigin.y);

			if ( Dx2 != 0 || Dz2 != 0 || Levels[Level].ShortC == true ) 
			{
				UpdateClipmap(Level, Dx2, Dz2); 
			}
		}
	}
}

void GPUGeometryClipmap::UpdateClipmap(int l, short Dx, short Dz)
{
	//LPDIRECT3DSURFACE9 pSurface = NULL, pBackBuffer = NULL;
	//renderer->changeToMainFramebuffer();	

	//ID3D11DeviceContext* pContext =  GetApp()->GetContext();

	//ID3D11RenderTargetView* pBackBufferRTV; 
	//ID3D11DepthStencilView* pBackBufferDSV;

	//pContext->OMGetRenderTargets(1, &pBackBufferRTV, &pBackBufferDSV);
	Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)GetApp()->getRenderer();
	
	RTT Vertex;
	RTTQuad Quads[12];
	VECTOR2 NewTexTorusOrigin;
	int NumQuads = 4;		
	short DX = 0, DY = 0;
	float TexOffsetX = 0.0f, TexOffsetY = 0.0f;

	//DXUTGetD3DDevice()->GetRenderTarget(0,&pBackBuffer);
	ID3D11RenderTargetView* pBackBufferRTV = Dx11R->getBackBuffer();

	NewTexTorusOrigin.x = Levels[l].TexTorusOrigin.x;
	NewTexTorusOrigin.y = Levels[l].TexTorusOrigin.y;

	if ( abs(Dx) < TexClipSize && abs(Dz) < TexClipSize && Levels[l].ShortC == false )
	{
		// Partial update
		short TX, TXX;
		short EndX1 = (short)abs(Dx);

		if ( Dx > 0 ) {
			DX = 1;
			TX = NP1;
			TXX = NP1 - Dx;
			if ( NewTexTorusOrigin.x >= TexClipSize ) { 
				NewTexTorusOrigin.x = NewTexTorusOrigin.x - TexClipSize; 
				EndX1 = Dx - NewTexTorusOrigin.x; 
				TX = TXX + EndX1;
			}
		} else {
			DX = -1;
			TX = NM1;
			TXX = NM1 + EndX1;
			if ( NewTexTorusOrigin.x <= 0 ) {
				NewTexTorusOrigin.x = NewTexTorusOrigin.x + TexClipSize;
				EndX1 = EndX1 - (EndX1 - Levels[l].LastTexTorusOrigin.x);
				TX = TXX - EndX1; 
			}
		}

		short X = Levels[l].LastTexTorusOrigin.x,
			XX = X + DX * EndX1;

		short TY = NP1 - NewTexTorusOrigin.y,
			TYY = NM1 + TexClipSize - NewTexTorusOrigin.y;

		Quads[NumQuads].Quad[0].SetXYUV( XX, 0, TX, TY );
		Quads[NumQuads].Quad[1].SetXYUV( X, 0, TXX, TY );
		Quads[NumQuads].Quad[2].SetXYUV( XX, NewTexTorusOrigin.y, TX, NP1 );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( X, NewTexTorusOrigin.y, TXX, NP1 );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		Quads[NumQuads].Quad[0].SetXYUV( XX, NewTexTorusOrigin.y, TX , NM1 );
		Quads[NumQuads].Quad[1].SetXYUV( X , NewTexTorusOrigin.y, TXX, NM1 );
		Quads[NumQuads].Quad[2].SetXYUV( XX, TexClipSize, TX, TYY );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( X , TexClipSize, TXX, TYY );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		short NXX = Levels[l].LastTexTorusOrigin.x - DX,
			NTX = TXX - DX;

		// Overwrite Last and First row of normal map
		Quads[0].Quad[0].SetXYUV( NXX, 0, NTX, TY );
		Quads[0].Quad[1].SetXYUV( X, 0, TXX, TY );
		Quads[0].Quad[2].SetXYUV( NXX, NewTexTorusOrigin.y, NTX, NP1 );
		Quads[0].Quad[3] = Quads[0].Quad[2];
		Quads[0].Quad[4].SetXYUV( X, NewTexTorusOrigin.y, TXX, NP1 );
		Quads[0].Quad[5] = Quads[0].Quad[1];

		Quads[1].Quad[0].SetXYUV( NXX, NewTexTorusOrigin.y, NTX , NM1 );
		Quads[1].Quad[1].SetXYUV( X , NewTexTorusOrigin.y, TXX, NM1 );
		Quads[1].Quad[2].SetXYUV( NXX, TexClipSize, NTX, TYY );
		Quads[1].Quad[3] = Quads[1].Quad[2];
		Quads[1].Quad[4].SetXYUV( X , TexClipSize, TXX, TYY );
		Quads[1].Quad[5] = Quads[1].Quad[1];

		if ( EndX1 != abs(Dx) )
		{
			// Cross x texture boundary
			X = DX > 0 ? 0 : TexClipSize; 

			TXX += DX * EndX1;
			EndX1 = (short)abs(Dx) - EndX1;

			XX = X + DX * EndX1;
			TX += DX * EndX1;

			Quads[NumQuads].Quad[0].SetXYUV( XX, 0, TX, TY );
			Quads[NumQuads].Quad[1].SetXYUV( X, 0, TXX, TY );
			Quads[NumQuads].Quad[2].SetXYUV( XX, NewTexTorusOrigin.y, TX , NP1 );
			Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
			Quads[NumQuads].Quad[4].SetXYUV( X , NewTexTorusOrigin.y, TXX, NP1 );
			Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
			NumQuads++;

			Quads[NumQuads].Quad[0].SetXYUV( XX, NewTexTorusOrigin.y, TX, NM1 );
			Quads[NumQuads].Quad[1].SetXYUV( X , NewTexTorusOrigin.y, TXX, NM1 );
			Quads[NumQuads].Quad[2].SetXYUV( XX, TexClipSize, TX, TYY );
			Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
			Quads[NumQuads].Quad[4].SetXYUV( X, TexClipSize, TXX, TYY );
			Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
			NumQuads++;
		}

		short EndY1 = (short)abs(Dz);

		if ( Dz > 0 ) {
			DY = 1;
			TY = NP1;
			TYY = NP1 - Dz ;
			if ( NewTexTorusOrigin.y >= TexClipSize ) { 
				NewTexTorusOrigin.y = NewTexTorusOrigin.y - TexClipSize; 
				EndY1 = Dz - NewTexTorusOrigin.y;
				TY = TYY + EndY1;
			}
		} else {
			DY = -1;
			TY = NM1;
			TYY = NM1 + EndY1 ;
			if ( NewTexTorusOrigin.y <= 0 ) {
				NewTexTorusOrigin.y = NewTexTorusOrigin.y + TexClipSize; 
				EndY1 = EndY1 - (EndY1 - Levels[l].LastTexTorusOrigin.y);
				TY = TYY - EndY1; 
			}
		}

		short Y = Levels[l].LastTexTorusOrigin.y,
			YY = Y + DY * EndY1;

		TX = NP1 - NewTexTorusOrigin.x;
		TXX = NM1 + TexClipSize - NewTexTorusOrigin.x;

		Quads[NumQuads].Quad[0].SetXYUV( 0, YY, TX, TY );
		Quads[NumQuads].Quad[1].SetXYUV( 0, Y, TX , TYY );
		Quads[NumQuads].Quad[2].SetXYUV( NewTexTorusOrigin.x, YY, NP1, TY );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( NewTexTorusOrigin.x, Y, NP1, TYY );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		Quads[NumQuads].Quad[0].SetXYUV( NewTexTorusOrigin.x, YY, NM1, TY );
		Quads[NumQuads].Quad[1].SetXYUV( NewTexTorusOrigin.x, Y, NM1, TYY );
		Quads[NumQuads].Quad[2].SetXYUV( TexClipSize        , YY, TXX , TY );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( TexClipSize        , Y, TXX , TYY );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		short NYY = Levels[l].LastTexTorusOrigin.y - DY,
			NTY = TYY - DY;

		// Overwrite Last and First col of normal map
		Quads[2].Quad[0].SetXYUV( 0, NYY, TX, NTY );
		Quads[2].Quad[1].SetXYUV( 0, Y, TX , TYY );
		Quads[2].Quad[2].SetXYUV( NewTexTorusOrigin.x, NYY, NP1, NTY );
		Quads[2].Quad[3] = Quads[2].Quad[2];
		Quads[2].Quad[4].SetXYUV( NewTexTorusOrigin.x, Y, NP1, TYY );
		Quads[2].Quad[5] = Quads[2].Quad[1];

		Quads[3].Quad[0].SetXYUV( NewTexTorusOrigin.x, NYY, NM1, NTY );
		Quads[3].Quad[1].SetXYUV( NewTexTorusOrigin.x, Y, NM1, TYY );
		Quads[3].Quad[2].SetXYUV( TexClipSize        , NYY, TXX , NTY );
		Quads[3].Quad[3] = Quads[3].Quad[2];
		Quads[3].Quad[4].SetXYUV( TexClipSize        , Y, TXX , TYY );
		Quads[3].Quad[5] = Quads[3].Quad[1];

		if ( EndY1 != abs(Dz) )
		{
			// Cross y texture boundary
			Y = DY > 0 ? 0 : TexClipSize; 

			TYY += DY * EndY1;
			EndY1 = (short)abs(Dz) - EndY1;

			YY = Y + DY * EndY1;
			TY += DY * EndY1;

			Quads[NumQuads].Quad[0].SetXYUV( 0, YY, TX , TY );
			Quads[NumQuads].Quad[1].SetXYUV( 0, Y , TX , TYY );
			Quads[NumQuads].Quad[2].SetXYUV( NewTexTorusOrigin.x, YY, NP1, TY );
			Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
			Quads[NumQuads].Quad[4].SetXYUV( NewTexTorusOrigin.x, Y, NP1, TYY );
			Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
			NumQuads++;

			Quads[NumQuads].Quad[0].SetXYUV( NewTexTorusOrigin.x, YY, NM1, TY );
			Quads[NumQuads].Quad[1].SetXYUV( NewTexTorusOrigin.x, Y, NM1, TYY );
			Quads[NumQuads].Quad[2].SetXYUV( TexClipSize        , YY, TXX , TY );
			Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
			Quads[NumQuads].Quad[4].SetXYUV( TexClipSize    , Y, TXX , TYY );
			Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
			NumQuads++;
		}
		Levels[l].LastTexTorusOrigin.x = NewTexTorusOrigin.x;
		Levels[l].LastTexTorusOrigin.y = NewTexTorusOrigin.y;

		Levels[l].TexTorusOrigin.x = NewTexTorusOrigin.x;
		Levels[l].TexTorusOrigin.y = NewTexTorusOrigin.y;
	} 
	else 
	{
		Levels[l].ShortC = false;
		// update the entire level if this level is rendered
		if ( Dx > 0 ) {
			while ( NewTexTorusOrigin.x >= TexClipSize ) { NewTexTorusOrigin.x = NewTexTorusOrigin.x - TexClipSize;	}
		} else {
			while ( NewTexTorusOrigin.x <= 0 ) { NewTexTorusOrigin.x = NewTexTorusOrigin.x + TexClipSize; }
		}

		if ( Dz > 0 ) {
			while ( NewTexTorusOrigin.y >= TexClipSize ) { NewTexTorusOrigin.y = NewTexTorusOrigin.y - TexClipSize; }
		} else {
			while ( NewTexTorusOrigin.y <= 0 ) { NewTexTorusOrigin.y = NewTexTorusOrigin.y + TexClipSize; }
		}

		short Ex,Ey,Bx,By;
		short Tx,Ty;

		Ex = NewTexTorusOrigin.x;
		Ey = NewTexTorusOrigin.y;

		Tx = NP1 - NewTexTorusOrigin.x ;
		Ty = NP1 - NewTexTorusOrigin.y ;

		Bx = 0;
		By = 0;

		Quads[NumQuads].Quad[0].SetXYUV( Ex, By, NP1, Ty );
		Quads[NumQuads].Quad[1].SetXYUV( Bx, By, Tx , Ty );
		Quads[NumQuads].Quad[2].SetXYUV( Ex, Ey, NP1, NP1);
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( Bx, Ey, Tx , NP1);
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		Bx = TexClipSize;
		By = 0;

		Quads[NumQuads].Quad[0].SetXYUV( Ex, By, NM1, Ty );
		Quads[NumQuads].Quad[1].SetXYUV( Bx, By, Tx , Ty );
		Quads[NumQuads].Quad[2].SetXYUV( Ex, Ey, NM1 , NP1 );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( Bx, Ey, Tx , NP1);
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		Bx = 0;
		By = TexClipSize;

		Quads[NumQuads].Quad[0].SetXYUV( Ex, By, NP1 , Ty );
		Quads[NumQuads].Quad[1].SetXYUV( Bx, By, Tx  , Ty );
		Quads[NumQuads].Quad[2].SetXYUV( Ex, Ey, NP1 , NM1 );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( Bx, Ey, Tx  , NM1 );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		Bx = TexClipSize;
		By = TexClipSize;

		Quads[NumQuads].Quad[0].SetXYUV( Ex, By, NM1 , Ty );
		Quads[NumQuads].Quad[1].SetXYUV( Bx, By, Tx  , Ty );
		Quads[NumQuads].Quad[2].SetXYUV( Ex, Ey, NM1 , NM1 );
		Quads[NumQuads].Quad[3] = Quads[NumQuads].Quad[2];
		Quads[NumQuads].Quad[4].SetXYUV( Bx, Ey, Tx  , NM1 );
		Quads[NumQuads].Quad[5] = Quads[NumQuads].Quad[1];
		NumQuads++;

		// Don't need to overwrite old first & last  row / col
		Quads[0].Quad[0].SetXYUV( -1, -1, -1, -1 );
		Quads[0].Quad[1] = Quads[0].Quad[2] = Quads[0].Quad[3] = Quads[0].Quad[4] = Quads[0].Quad[5] = Quads[0].Quad[0];

		Quads[1].Quad[0] = Quads[1].Quad[1] = Quads[1].Quad[2] = Quads[1].Quad[3] = Quads[1].Quad[4] = Quads[1].Quad[5] = Quads[0].Quad[0];
		Quads[2].Quad[0] = Quads[2].Quad[1] = Quads[2].Quad[2] = Quads[2].Quad[3] = Quads[2].Quad[4] = Quads[2].Quad[5] = Quads[0].Quad[0];
		Quads[3].Quad[0] = Quads[3].Quad[1] = Quads[3].Quad[2] = Quads[3].Quad[3] = Quads[3].Quad[4] = Quads[3].Quad[5] = Quads[0].Quad[0];

		Levels[l].LastTexTorusOrigin.x = NewTexTorusOrigin.x;
		Levels[l].LastTexTorusOrigin.y = NewTexTorusOrigin.y;

		Levels[l].TexTorusOrigin.x = NewTexTorusOrigin.x;
		Levels[l].TexTorusOrigin.y = NewTexTorusOrigin.y;
	}

	// Update clipmap	
	context->OMSetRenderTargets(0, 1, &Levels[l].m_ptex2DElevDataRTV, NULL);

	/*D3DXHANDLE hTech;
	if (FAILED(m_pUpdate->FindNextValidTechnique(NULL,&hTech)))
	{
		exit(-1);
	}*/
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pUpdateTec->GetDesc( &techDesc );

	
	pd3dDevice->IASetInputLayout( m_pUpdateLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = g_pParticleBuffer;
	Strides[0] = sizeof( RTT );
	Offsets[0] = sizeof(RTT) * 6 * 4;
		 
	pd3dDevice->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	D3D11Dev()->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	D3D11Dev()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	
		if (l+1 > L-1) 
		{
			//Pass = 1;
			noVec4 Cto = noVec4(
				TexOffsetX,
				TexOffsetY,
				0.0f,
				0.0f);
						
			m_pCoarserTexOffsetVar->SetFloatVector((float*)&Cto);
			m_pOneOverTextureSizeVar->SetFloat((float)TexelClip);
			m_pTexScale->SetFloatVector((float*)&Levels[I].VecScale);			
			m_pFloatPrecisonVar->SetFloat((float)FloatPrecision);
			m_pClipmapRTTVar->SetMatrix((float*)&m_mClipmapRTT);
			m_ptCoarserElevationSRVVar->SetResource(Levels[min(l+1,L-1)]->GetElevDataRTV());
			m_pPermTexSRVVar->SetResource(m_ptex2DPermSRV);
			m_pGradTexSRVVar->SetResource(m_ptex2DGradSRV);

			m_pUpdateTec->GetPassByIndex( 1 )->Apply(0, context);
			D3D11Dev()->Draw((NumQuads - 4) * 2 * 3, 0);			
		}
		else 
		{
			// Coarser tex offset for coarser value
			TexOffsetX = TexCoarserOffset[(l%2)] + (Levels[l].TexScale.x * 0.5f);
			TexOffsetY = TexCoarserOffset[(l%2)] + (Levels[l].TexScale.y * 0.5f);

			// Coarser tex offset for coarser value
			TexOffsetX = TexCoarserOffset[(l%2)] + (Levels[l].TexScale.x * 0.5f);
			TexOffsetY = TexCoarserOffset[(l%2)] + (Levels[l].TexScale.y * 0.5f);

			//m_pUpdate->SetTexture("tCoarserElevation", Levels[min(l+1,L-1)].ElevationMap);
			noVec4 Cto = noVec4(
				TexOffsetX,
				TexOffsetY,
				0.0f,
				0.0f);

			m_pCoarserTexOffsetVar->SetFloatVector((float*)&Cto);
			m_pOneOverTextureSizeVar->SetFloat((float)TexelClip);
			m_pTexScale->SetFloatVector((float*)&Levels[I].VecScale);			
			m_pFloatPrecisonVar->SetFloat((float)FloatPrecision);
			m_pClipmapRTTVar->SetMatrix((float*)&m_mClipmapRTT);
			m_ptCoarserElevationSRVVar->SetResource(Levels[min(l+1,L-1)]->GetElevDataRTV());
			m_pPermTexSRVVar->SetResource(m_ptex2DPermSRV);
			m_pGradTexSRVVar->SetResource(m_ptex2DGradSRV);
						

			m_pUpdateTec->GetPassByIndex( 0 )->Apply(0, context);

			D3D11Dev()->Draw((NumQuads - 4) * 2 * 3, 0);			
		}		
	}

	// Update normal	
	/*Levels[l].NormalMap->GetSurfaceLevel(0,&pSurface);
	DXUTGetD3DDevice()->SetRenderTarget(0, pSurface);*/
	context->OMSetRenderTargets(0, 1, &Levels[l].GetNormalMapRTV(), NULL);
		
	m_pElevationSRVVar->SetTexture(Levels[l].GetElevDataRTV);
	m_pElevationSRVVar->SetTexture( Levels[min(l+1,L-1)].GetNormalMapRTV());

	noVec4 Cto = noVec4(
		TexOffsetX,
		TexOffsetY,
		0,
		0);


	noVec4 ScaleFac = noVec4 ( -0.5f * ZScaleOverFp / Levels[l].VecScale.x, -0.5f * ZScaleOverFp / Levels[l].VecScale.y, 0.0f, 0.0f ); 

	m_pCoarserTexOffsetVar2->SetVector(&Cto);	
	m_pScaleFac->SetVector(&ScaleFac);
	m_pTexelClipVar2->SetFloat(TexelClip);
	m_pZScaleOverFp->SetFloat(ZScaleOverFp);
	m_pClipmapRTTVar2->SetMatrix(&m_mClipmapRTT);
	
	// Reuse the same quads as for elevationmaps + 4 quads to overwrite first & last row/col 
	
	//DXUTGetD3DDevice()->SetVertexDeclaration( m_pUpdateDeclaration );

	pd3dDevice->IASetInputLayout( m_pComputeNormalLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = g_pParticleBuffer;
	Strides[0] = sizeof( RTT );
	Offsets[0] = 0;

	pd3dDevice->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	D3D11Dev()->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	D3D11Dev()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	NumQuads * 2, , sizeof(RTT) );
		
	if ( l+1 > L-1 )
	{
		m_pUpdateTec->GetPassByIndex( 1 )->Apply(0, context);

		D3D11Dev()->Draw(NumQuads * 2 * 3, 0);		
	}
	else
	{
		
		m_pUpdateTec->GetPassByIndex( 0 )->Apply(0, context);

		D3D11Dev()->Draw(NumQuads * 2 * 3, 0);		

	}		
	
	renderer->changeToMainFramebuffer();
	
}

void LoadEffects()
{
	//const DWORD Flags = D3DXSHADER_IEEE_STRICTNESS | D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_NO_PRESHADER;
	//const DWORD Flags = D3DXSHADER_OPTIMIZATION_LEVEL3;

	loadTextureFromFile("Data/Shaders/Render.fx", );
	loadTextureFromFile("Data/Shaders/Update.fx", );
	loadTextureFromFile("Data/Shaders/ComputeNormals.fx", );
}


void Create(char Level, float LastShift)
{
	if ( Level < L )
	{
		const float GridSpacing = powf(2,Level);
		const float Shifting = GridShifting(Level, LastShift);

		Levels[Level].Hpos = Levels[Level].Vpos = Level%2;
		Levels[Level].VecShift = powf( 2.0f, (float)(Level + 1) );
		Levels[Level].VecScale = noVec4( GridSpacing, GridSpacing, -(N/2) * GridSpacing + Shifting, -(N/2) * GridSpacing + Shifting );
		Levels[Level].AABBDiff = noVec2( (M-1) * GridSpacing, (M-1) * GridSpacing);
		Levels[Level].TexScale = noVec2( 0.0f, 0.0f );
		Levels[Level].TexTorusOrigin.x = Levels[Level].TexTorusOrigin.y =
			Levels[Level].LastTexTorusOrigin.x = Levels[Level].LastTexTorusOrigin.y = NP1;
		Levels[Level].ShortC = false;

		// Create elevation map
		if ( FAILED( D3DXCreateTexture(DXUTGetD3DDevice(),
			TexClipSize, TexClipSize,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F, D3DPOOL_DEFAULT,
			&Levels[Level].ElevationMap) )
			)
		{
			exit(-1);
		}

		// Create Normal map
		if ( FAILED( D3DXCreateTexture(DXUTGetD3DDevice(),
			TexNormalSize, TexNormalSize,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
			&Levels[Level].NormalMap) )
			)
		{
			exit(-1);
		}

		Create(Level + 1, Shifting);
	}
}

void CreateRTTMatrix()
{
	D3DXMatrixOrthoOffCenterLH( &m_mClipmapRTT, 0.0f, (float)TexClipSize, (float)TexClipSize, 0.0f, 0.0f, 1.0f );
}

void CreateNoise()
{
	if ( FAILED( D3DXCreateTexture(
		DXUTGetD3DDevice(),
		256,
		256,
		1,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&PermTexture
		) ) )
	{
		exit(-1);
	}

	if (FAILED (D3DXFillTexture (PermTexture, FillPermTexture, NULL)))
	{
		exit(-1);
	}

	if ( FAILED ( D3DXCreateTexture(
		DXUTGetD3DDevice(),
		256,
		256,
		1,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&GradTexture
		) ) )
	{
		exit(-1);
	}

	if (FAILED ( D3DXFillTexture (GradTexture, FillGradTexture, NULL)))
	{
		exit(-1);
	}
}

void CreateGeometry()
{
	CreateVertexBuffer();
	CreateInstanceBuffer();
	CreateMxMIndexStrip();
	CreateMx3IndexStrip();
	CreateLShapeIndexStrip(0);
	CreateLShapeIndexStrip(1);
	CreateLShapeIndexStrip(2);
	CreateLShapeIndexStrip(3);
	CreateOuterDegenerateTrianglesIndexList();
}

void CreateVertexBuffer()
{
	// Create vertex buffer
	if( FAILED( DXUTGetD3DDevice()->CreateVertexBuffer( (N * N) * 2 * sizeof(short), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,
		&m_pVertexBuffer, NULL ) ) )
	{
		exit(-1);
	}

	// Lock the vertex buffer
	short * pVertices;
	if( FAILED( m_pVertexBuffer->Lock(0, 0, (VOID**)&pVertices, 0 ) ) )
	{
		exit(-1);
	}

	// 0 .. N
	for( short i = 0; i < N; i++)
	{
		for( short j = 0; j < N; j++)
		{	
			// Fill vertex buffer
			pVertices[i * (N * 2) + j * 2]    = i;
			pVertices[i * (N * 2) + j * 2+ 1] = j;
		}
	}

	m_pVertexBuffer->Unlock();
}

void CreateInstanceBuffer()
{
	// Create instance buffer
	if( FAILED( DXUTGetD3DDevice()->CreateVertexBuffer( (L * 12 + 4 + L) * (8 * sizeof(float)), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,
		&m_pInstanceBuffer, NULL ) ) )
	{
		exit(-1);
	}

	// Lock instance buffer
	noVec4 * pVertices;
	if( FAILED( m_pInstanceBuffer->Lock(0, 0, (VOID**)&pVertices, 0 ) ) )
	{
		exit(-1);
	}

	// Fill instance buffer
	for( short i = 0; i < (L * 12 + 4 + L) * 2; i++)
	{
		pVertices[i] = noVec4( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	m_pInstanceBuffer->Unlock();
}

void CreateMxMIndexStrip()
{
	LPD3DXMESH Grid;

	// Create Mesh
	D3DXCreateMeshFVF(
		( M - 1 ) * ( M - 1 ) * 2,
		N * N,
		0,
		D3DFVF_XYZ,
		DXUTGetD3DDevice(),
		&Grid);

	// Lock Mesh vertex buffer
	D3DXVECTOR3 * pVerticesMesh;
	if( FAILED( Grid->LockVertexBuffer(0, (VOID**)&pVerticesMesh ) ) )
	{
		exit(-1);
	}

	// Fill 0 .. N
	for( short i = 0; i < N; i++)
	{
		for( short j = 0; j < N; j++)
		{	
			pVerticesMesh[i * N + j].x = (float)i;
			pVerticesMesh[i * N + j].y = 0.0f;
			pVerticesMesh[i * N + j].z = (float)j;
		}
	}

	Grid->UnlockVertexBuffer();

	// Lock Mesh index buffer
	unsigned short * pIndicesMesh;
	if( FAILED( Grid->LockIndexBuffer( 0, (VOID**)&pIndicesMesh ) ) )
	{
		exit(-1);
	}

	for ( short i = 0; i < M-1; i++) 
	{
		for ( short j = 0; j < M-1; j++)
		{
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = i * N + (j + 1);
			*(pIndicesMesh++) = (i+1) * N + (j + 1);

			*(pIndicesMesh++) = (i+1) * N + j;
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = (i+1) * N + (j + 1);
		}
	}

	Grid->UnlockIndexBuffer();

	// Optimize mesh
	int NumFace = Grid->GetNumFaces() * sizeof(short) * 3;
	DWORD * Ad = new DWORD [NumFace];
	Grid->GenerateAdjacency(0.0f, Ad);

	Grid->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_IGNOREVERTS,
		Ad,
		NULL,
		NULL,
		NULL);

	delete [] Ad;

	// Convert to TriStrip
	D3DXConvertMeshSubsetToSingleStrip(Grid,
		0,
		D3DXMESH_IB_WRITEONLY,
		&m_pIndexBufferMxM,
		&NumVerticesMxMIndex
		);

	PrimitiveCountMxMIndex = NumVerticesMxMIndex - 2;

	SAFE_RELEASE( Grid );
}

void CreateMx3IndexStrip()
{
	LPD3DXMESH Grid;

	// Create Mesh
	D3DXCreateMeshFVF(
		( (M - 1) * ((M * 2 ) - (M * 2 - 2)) ) * 2 * 4,
		N * N,
		0,
		D3DFVF_XYZ,
		DXUTGetD3DDevice(),
		&Grid);

	// Lock Mesh vertex buffer
	D3DXVECTOR3 * pVerticesMesh;
	if( FAILED( Grid->LockVertexBuffer(0, (VOID**)&pVerticesMesh ) ) )
	{
		exit(-1);
	}

	// Fill 0 .. N
	for( short i = 0; i < N; i++)
	{
		for( short j = 0; j < N; j++)
		{	
			pVerticesMesh[i * N + j].x = (float)i;
			pVerticesMesh[i * N + j].y = 0.0f;
			pVerticesMesh[i * N + j].z = (float)j;
		}
	}

	Grid->UnlockVertexBuffer();

	// Lock Mesh index buffer
	unsigned short * pIndicesMesh;
	if( FAILED( Grid->LockIndexBuffer( 0, (VOID**)&pIndicesMesh ) ) )
	{
		exit(-1);
	}

	for ( short i = 0; i < M - 1; i++) 
	{
		for ( short j = (M * 2 - 2); j < (M * 2); j++)
		{
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = i * N + (j + 1);
			*(pIndicesMesh++) = (i+1) * N + (j + 1);

			*(pIndicesMesh++) = (i+1) * N + j;
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = (i+1) * N + (j + 1);
		}
	}

	for ( short j = 0; j < M - 1; j++) 
	{
		for ( short i = (M * 2 - 2); i < (M * 2); i++)
		{
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = i * N + (j + 1);
			*(pIndicesMesh++) = (i+1) * N + (j + 1);

			*(pIndicesMesh++) = (i+1) * N + j;
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = (i+1) * N + (j + 1);
		}
	}

	for ( short i = (N - 1) - (M - 1); i < (N - 1); i++) 
	{
		for ( short j = (M * 2 - 2); j < (M * 2); j++)
		{
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = i * N + (j + 1);
			*(pIndicesMesh++) = (i+1) * N + (j + 1);

			*(pIndicesMesh++) = (i+1) * N + j;
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = (i+1) * N + (j + 1);
		}
	}

	for ( short j = (N - 1) - (M - 1); j < (N - 1); j++) 
	{
		for ( short i = (M * 2 - 2); i < (M * 2); i++)
		{
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = i * N + (j + 1);
			*(pIndicesMesh++) = (i+1) * N + (j + 1);

			*(pIndicesMesh++) = (i+1) * N + j;
			*(pIndicesMesh++) = i * N + j;
			*(pIndicesMesh++) = (i+1) * N + (j + 1);
		}
	}

	Grid->UnlockIndexBuffer();

	// Optimize mesh
	int NumFace = Grid->GetNumFaces() * sizeof(short) * 3;
	DWORD * Ad = new DWORD [NumFace];
	Grid->GenerateAdjacency(0.0f, Ad);

	Grid->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_IGNOREVERTS,
		Ad,
		NULL,
		NULL,
		NULL);

	delete [] Ad;

	// Convert to TriStrip
	D3DXConvertMeshSubsetToSingleStrip(Grid,
		0,
		D3DXMESH_IB_WRITEONLY,
		&m_pIndexBufferMx3,
		&NumVerticesMx3Index
		);

	PrimitiveCountMx3Index = NumVerticesMx3Index - 2;
	SAFE_RELEASE( Grid );
}

void CreateLShapeIndexStrip(int ls)
{
	LPD3DXMESH Grid;

	// TODO : Remove the 2 or 4 vertices in double in index buffers 
	// Create Mesh
	D3DXCreateMeshFVF(
		(((N - 1) - (M -1)) - (M - 1)) * 4,
		N * N,
		0,
		D3DFVF_XYZ,
		DXUTGetD3DDevice(),
		&Grid);

	// Lock Mesh vertex buffer
	D3DXVECTOR3 * pVerticesMesh;
	if( FAILED( Grid->LockVertexBuffer(0, (VOID**)&pVerticesMesh ) ) )
	{
		exit(-1);
	}

	// Fill 0 .. N
	for( short i = 0; i < N; i++)
	{
		for( short j = 0; j < N; j++)
		{	
			pVerticesMesh[i * N + j].x = (float)i;
			pVerticesMesh[i * N + j].y = 0.0f;
			pVerticesMesh[i * N + j].z = (float)j;
		}
	}

	Grid->UnlockVertexBuffer();

	// Lock Mesh index buffer
	unsigned short * pIndicesMesh;
	if( FAILED( Grid->LockIndexBuffer( 0, (VOID**)&pIndicesMesh ) ) )
	{
		exit(-1);
	}

	switch ( ls )
	{
	case 0 :
		for ( short i = M - 1; i < M ; i++) 
		{
			for ( short j = M - 1; j < ((N - 1) - (M -1)); j++)
			{
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = i * N + (j + 1);
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);

				*(pIndicesMesh++) = (i + 1) * N + j;
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);
			}
		}

		for ( short i = (N - 1) - (M - 1) - 1; i >=(M - 1); i--) 
		{
			for ( short j = (N - 1) - (M - 1) - 1; j >= (N - 1) - M; j--)
			{
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = i * N + (j + 1);
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);

				*(pIndicesMesh++) = (i + 1) * N + j;
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);
			}
		}
		break;

	case 1 :
		for ( short i = (N - 1) - (M - 1) - 1; i >=(M - 1); i--) 
		{
			for ( short j = (N - 1) - (M - 1) - 1; j >= (N - 1) - M; j--)
			{
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = i * N + (j + 1);
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);

				*(pIndicesMesh++) = (i + 1) * N + j;
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);
			}
		}

		for ( short i = (N - 1) - (M - 1) - 1; i >=(M - 1); i--) 
		{
			for ( short j = (N - 1) - (M - 1) - 1; j >= (N - 1) - M; j--)
			{
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = j * N + (i + 1);
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);

				*(pIndicesMesh++) = (j + 1) * N + i;
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);
			}
		}
		break;

	case 2 :
		for ( short i = M - 1; i < M ; i++) 
		{
			for ( short j = M - 1; j < ((N - 1) - (M -1)); j++)
			{
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = j * N + (i + 1);
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);

				*(pIndicesMesh++) = (j + 1) * N + i;
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);
			}
		}

		for ( short i = (N - 1) - (M - 1) - 1; i >=(M - 1); i--) 
		{
			for ( short j = (N - 1) - (M - 1) - 1; j >= (N - 1) - M; j--)
			{
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = j * N + (i + 1);
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);

				*(pIndicesMesh++) = (j + 1) * N + i;
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);
			}
		}
		break;

	case 3 :
		for ( short i = M - 1; i < M ; i++) 
		{
			for ( short j = M - 1; j < ((N - 1) - (M -1)); j++)
			{
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = j * N + (i + 1);
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);

				*(pIndicesMesh++) = (j + 1) * N + i;
				*(pIndicesMesh++) = j * N + i;
				*(pIndicesMesh++) = (j + 1) * N + (i + 1);
			}
		}

		for ( short i = M - 1; i < M ; i++) 
		{
			for ( short j = M - 1; j < ((N - 1) - (M -1)); j++)
			{
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = i * N + (j + 1);
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);

				*(pIndicesMesh++) = (i + 1) * N + j;
				*(pIndicesMesh++) = i * N + j;
				*(pIndicesMesh++) = (i + 1) * N + (j + 1);
			}
		}
		break;
	}

	Grid->UnlockIndexBuffer();

	// Optimize mesh
	int NumFace = Grid->GetNumFaces() * sizeof(short) * 3;
	DWORD * Ad = new DWORD [NumFace];
	Grid->GenerateAdjacency(0.0f, Ad);

	Grid->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_IGNOREVERTS,
		Ad,
		NULL,
		NULL,
		NULL);

	delete [] Ad;

	// Convert to TriStrip
	D3DXConvertMeshSubsetToSingleStrip(Grid,
		0,
		D3DXMESH_IB_WRITEONLY,
		&m_pIndexBufferLShape[ls],
		&NumVerticesLShapeIndex[ls]
	);

	PrimitiveCountLShapeIndex[ls] = NumVerticesLShapeIndex[ls] - 2;
	SAFE_RELEASE( Grid );
}

void CreateOuterDegenerateTrianglesIndexList()
{
	if( FAILED( DXUTGetD3DDevice()->CreateIndexBuffer( (((N-1) / 2) * 3) * 4 * sizeof(WORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, 
		&m_pIndexBufferOuterDegenerateTriangles, NULL ) ) )
	{
		exit(-1);
	}

	unsigned short * pIndicesMesh;
	if( FAILED( m_pIndexBufferOuterDegenerateTriangles->Lock( 
		0,                 
		0, 
		(VOID**)&pIndicesMesh,  
		0 ) ) )  
	{
		exit(-1);
	}

	for ( unsigned short i = 0; i < N - 2; i+=2 )
	{
		*(pIndicesMesh++) = i;
		*(pIndicesMesh++) = i + 2;
		*(pIndicesMesh++) = i + 1;
	}

	for ( unsigned short i = 0; i < N - 2; i+=2 )
	{
		*(pIndicesMesh++) = i * N + (N-1);
		*(pIndicesMesh++) = (i + 2) * N + (N-1);
		*(pIndicesMesh++) = (i + 1) * N + (N-1);
	}

	for ( unsigned short i =  N-1; i > 1; i-=2 )
	{
		*(pIndicesMesh++) = (N-1) * N + i;
		*(pIndicesMesh++) = (N-1) * N + (i - 2);
		*(pIndicesMesh++) = (N-1) * N + (i - 1);
	}

	for ( unsigned short i = N-1; i > 1; i-=2 )
	{
		*(pIndicesMesh++) = i * N;
		*(pIndicesMesh++) = (i - 2) * N;
		*(pIndicesMesh++) = (i - 1) * N;
	}

	m_pIndexBufferOuterDegenerateTriangles->Unlock();

	NumVerticesOuterDegenerateTrianglesIndex = (((N-1) / 2) * 3) * 4;
	PrimitiveCountOuterDegenerateTrianglesIndex = NumVerticesOuterDegenerateTrianglesIndex / 3;
}

noVec4 * CullBlock(int n, int l, noVec4 * pVertices )
{
	//Create Box
	noVec2 Min, Max;
	Min.x = Levels[l].VecScale.z + BlockOrigin[n].x * Levels[l].VecScale.x;
	Min.y = Levels[l].VecScale.w + BlockOrigin[n].y * Levels[l].VecScale.y;
	Max = Min + Levels[l].AABBDiff;

	// Check box vs Frustum
	int In, i = 0;

	D3DXVECTOR3 Corners[8];
	Corners[0] = D3DXVECTOR3((Min.x), (ZScale), (Max.y));
	Corners[1] = D3DXVECTOR3((Max.x), (ZScale), (Max.y));
	Corners[2] = D3DXVECTOR3((Max.x), (0.0f), (Max.y));
	Corners[3] = D3DXVECTOR3((Min.x), (0.0f), (Max.y));
	Corners[4] = D3DXVECTOR3((Min.x), (ZScale), (Min.y));
	Corners[5] = D3DXVECTOR3((Max.x), (ZScale), (Min.y));
	Corners[6] = D3DXVECTOR3((Max.x), (0.0f), (Min.y));
	Corners[7] = D3DXVECTOR3((Min.x), (0.0f), (Min.y));

	while (i < 6) 
	{ 
		In = 8;

		for (int j = 0; j < 8; j++) 
			if ( D3DXPlaneDotCoord(&FrustumPlanes[i],&Corners[j]) < 0.0f ) In--;

		i++;
		if ( In == 0 ) i = 10;
	} 

	// In
	if ( i == 6 )
	{
		noVec2 v2TexScale = Levels[l].TexScale + BlockOrigin[n] * TexelClip;
		*(pVertices++) = noVec4( Levels[l].VecScale.x, Levels[l].VecScale.y, Min.x, Min.y );
		*(pVertices++) = noVec4( TexelClip, TexelClip, v2TexScale.x, v2TexScale.y );
		Levels[l].NumInstances++; 
	}
	return pVertices;
}

noVec4 * CullBlockFinest(int n, noVec4 * pVertices)
{
	//Create Box
	noVec2 Min, Max;
	Min.x = Levels[MinLevel].VecScale.z + BlockOriginFinest[n].x * Levels[MinLevel].VecScale.x;
	Min.y = Levels[MinLevel].VecScale.w + BlockOriginFinest[n].y * Levels[MinLevel].VecScale.y;
	Max = Min + Levels[MinLevel].AABBDiff;

	// Check box vs Frustum
	int In, i = 0;

	D3DXVECTOR3 Corners[8];
	Corners[0] = D3DXVECTOR3((Min.x), (ZScale), (Max.y));
	Corners[1] = D3DXVECTOR3((Max.x), (ZScale), (Max.y));
	Corners[2] = D3DXVECTOR3((Max.x), (0.0f), (Max.y));
	Corners[3] = D3DXVECTOR3((Min.x), (0.0f), (Max.y));
	Corners[4] = D3DXVECTOR3((Min.x), (ZScale), (Min.y));
	Corners[5] = D3DXVECTOR3((Max.x), (ZScale), (Min.y));
	Corners[6] = D3DXVECTOR3((Max.x), (0.0f), (Min.y));
	Corners[7] = D3DXVECTOR3((Min.x), (0.0f), (Min.y));

	while (i < 6) 
	{ 
		In = 8;

		for (int j = 0; j < 8; j++) 
			if ( D3DXPlaneDotCoord(&FrustumPlanes[i],&Corners[j]) < 0.0f ) In--;

		i++;
		if ( In == 0 ) i = 10;
	} 

	// In
	if ( (i == 6) )
	{
		noVec2 v2TexScale = Levels[MinLevel].TexScale + BlockOriginFinest[n] * TexelClip;
		*(pVertices++) = noVec4( Levels[MinLevel].VecScale.x, Levels[MinLevel].VecScale.y, Min.x, Min.y );
		*(pVertices++) = noVec4( TexelClip, TexelClip, v2TexScale.x, v2TexScale.y );
		NumInstancesFinest++; 
	}
	return pVertices;
}

public :
	GPUGeometryClipmap(short n = 31, short l = 6): N(n), L(l), M( (n + 1) / 4 )
	{
		assert(l > 0);
		assert(n > 6);
		Levels = new Clipmap[L];

		// Init
		TexClipSize = (n + 1);
		TexelClip = (float)(1.0f / TexClipSize);
		TexNormalSize = TexClipSize * 1;
		TexClipShift = TexelClip * 2.0f;
		TexCoarserOffset[0] = (float)(M - 0.5f) * TexelClip;
		TexCoarserOffset[1] = (float)(M + 0.5f) * TexelClip;
		Viewer = LastViewer = D3DXVECTOR3 ( 0.0f, 0.0f, 0.0f );
		ZScale = 1024.0f;
		FloatPrecision = ZScale * 20;
		OneOverFp = 1.0f / FloatPrecision;
		ZScaleOverFp = ZScale / FloatPrecision;
		MinLevel = 0;

		NP1 = (short)(N + floor((TexClipSize - (N + 1)) * 0.5f));
		NM1 = (short)-(TexClipSize - NP1);

		BlockOrigin[0]  = noVec2( 0.0f					  , 0.0f        );
		BlockOrigin[1]  = noVec2( (float)(M - 1)		  , 0.0f        );
		BlockOrigin[2]  = noVec2( (float)(M * 2)		  , 0.0f        );
		BlockOrigin[3]  = noVec2( (float)(M * 3 - 1)	  , 0.0f        );

		BlockOrigin[4]  = noVec2( (float)(M * 3 - 1)	  , (float)(M - 1)     );
		BlockOrigin[5]  = noVec2( (float)(M * 3 - 1)	  , (float)(M * 2)     );
		BlockOrigin[6]  = noVec2( (float)(M * 3 - 1)	  , (float)(M * 3 - 1) );

		BlockOrigin[7]  = noVec2( (float)(M * 2)		  , (float)(M * 3 - 1) );
		BlockOrigin[8]  = noVec2( (float)(M - 1)		  , (float)(M * 3 - 1) );
		BlockOrigin[9]  = noVec2( 0.0f					  , (float)(M * 3 - 1) );

		BlockOrigin[10] = noVec2( 0.0f					  , (float)(M * 2)     );
		BlockOrigin[11] = noVec2( 0.0f					  , (float)(M - 1)     );

		BlockOriginFinest[0] = noVec2( (float)(M - 0)    , (float)(M - 0)     );
		BlockOriginFinest[1] = noVec2( (float)(M * 2 - 1), (float)(M - 0)     );
		BlockOriginFinest[2] = noVec2( (float)(M - 0)    , (float)(M * 2 - 1) );
		BlockOriginFinest[3] = noVec2( (float)(M * 2 - 1), (float)(M * 2 - 1) );

		// Init finest level
		Levels[0].Hpos = 0;
		Levels[0].Vpos = 0;
		Levels[0].VecShift = 2.0f;
		Levels[0].VecScale = noVec4( 1.0f, 1.0f, (float)(-(N/2)), (float)(-(N/2)) );
		Levels[0].TexScale = noVec2( 0.0f, 0.0f );
		Levels[0].AABBDiff = noVec2( (float)(M-1), (float)(M-1) );
		Levels[0].TexTorusOrigin.x = Levels[0].TexTorusOrigin.y = 
			Levels[0].LastTexTorusOrigin.x = Levels[0].LastTexTorusOrigin.y = NP1;
		Levels[0].ShortC = false;

		Noise(float)->setNoise( new CSimplexPerlin<float> );

		CreateGeometry();

		// Create elevation map
		if ( FAILED( D3DXCreateTexture(DXUTGetD3DDevice(),
			TexClipSize, TexClipSize,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F, D3DPOOL_DEFAULT,
			&Levels[0].ElevationMap) )
			)
		{
			exit(-1);
		}

		// Create Normal map
		if ( FAILED( D3DXCreateTexture(DXUTGetD3DDevice(),
			TexNormalSize, TexNormalSize,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
			&Levels[0].NormalMap) )
			)
		{
			exit(-1);
		}

		// Create Vertex Declaration
		const D3DVERTEXELEMENT9 g_VBDecl_Geometry[] =
		{
			{0,  0, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{1,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			{1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
			D3DDECL_END()
		};

		DXUTGetD3DDevice()->CreateVertexDeclaration(g_VBDecl_Geometry, &m_pVertexDeclaration);

		// Create Update Declaration
		const D3DVERTEXELEMENT9 g_UpdateDecl_Geometry[] =
		{
			{0,  0, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0,  4, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			D3DDECL_END()
		};

		DXUTGetD3DDevice()->CreateVertexDeclaration(g_UpdateDecl_Geometry, &m_pUpdateDeclaration);

		LoadEffects();
		Create(1, 1.0f);
		CreateNoise();
		CreateElevation();
	}

	void CreateElevation( )
	{
		LPDIRECT3DSURFACE9 pSurface = NULL, pBackBuffer = NULL;
		D3DXHANDLE hTech;
		RTT Vertex;
		RTTQuad Quads[4];		

		CreateRTTMatrix();
		DXUTGetD3DDevice()->GetRenderTarget(0,&pBackBuffer);

		for (int l = L-1; l >= 0; l-- )
		{
			// Clipmap Creation
			if (FAILED(m_pUpdate->FindNextValidTechnique(NULL,&hTech)))
			{
				exit(-1);
			}

			if (SUCCEEDED(m_pUpdate->SetTechnique(hTech))) 
			{
				unsigned int cPasses = 0;
				if (SUCCEEDED(m_pUpdate->Begin(&cPasses, 0))) 
				{
					m_pUpdate->SetTexture("tpermTexture", PermTexture);
					m_pUpdate->SetTexture("tgradTexture", GradTexture);
					m_pUpdate->SetTexture("tTerrain", G1);

					float TexOffset = 0.0f;

					Levels[l].ElevationMap->GetSurfaceLevel(0,&pSurface);
					DXUTGetD3DDevice()->SetRenderTarget(0, pSurface);

					if ( l+1 > L-1 )
					{
						m_pUpdate->BeginPass(1);
					}
					else
					{
						TexOffset = TexCoarserOffset[(l%2)];
						m_pUpdate->BeginPass(0);
					}

					noVec4 Cto = noVec4(
						TexOffset,
						TexOffset,
						0,
						0);

					m_pUpdate->SetVector("CoarserTexOffset", &Cto);
					m_pUpdate->SetVector("TexScale", &Levels[l].VecScale);
					m_pUpdate->SetFloat("OneOverTextureSize", TexelClip);
					m_pUpdate->SetFloat("FloatPrecision", FloatPrecision );
					m_pUpdate->SetTexture("tCoarserElevation", Levels[min(l+1,L-1)].ElevationMap);
					m_pUpdate->SetMatrix("Proj", &m_mClipmapRTT);
					m_pUpdate->CommitChanges();

					short Ex,Ey,Bx,By;

					Ex = NP1;
					Ey = NP1;

					Bx = 0;
					By = 0;

					Quads[0].Quad[0].SetXYUV( Ex, By, NP1, 0 );
					Quads[0].Quad[1].SetXYUV( Bx, By, 0, 0 );
					Quads[0].Quad[2].SetXYUV( Ex, Ey, NP1, NP1 );
					Quads[0].Quad[3] = Quads[0].Quad[2];
					Quads[0].Quad[4].SetXYUV( Bx, Ey, 0, NP1 );
					Quads[0].Quad[5] = Quads[0].Quad[1];

					Bx = TexClipSize;
					By = 0;

					Quads[1].Quad[0].SetXYUV( Ex, By, NM1, 0);
					Quads[1].Quad[1].SetXYUV( Bx, By, 0, 0);
					Quads[1].Quad[2].SetXYUV( Ex, Ey, NM1, NP1);
					Quads[1].Quad[3] = Quads[1].Quad[2];
					Quads[1].Quad[4].SetXYUV( Bx, Ey, 0, NP1 );
					Quads[1].Quad[5] = Quads[1].Quad[1];

					Bx = 0;
					By = TexClipSize;

					Quads[2].Quad[0].SetXYUV( Ex, By, NP1, 0 );
					Quads[2].Quad[1].SetXYUV( Bx, By, 0, 0 );
					Quads[2].Quad[2].SetXYUV( Ex, Ey, NP1, NM1 );
					Quads[2].Quad[3] = Quads[2].Quad[2];
					Quads[2].Quad[4].SetXYUV( Bx, Ey, 0, NM1 );
					Quads[2].Quad[5] = Quads[2].Quad[1];

					Bx = TexClipSize;
					By = TexClipSize;

					Quads[3].Quad[0].SetXYUV( Ex, By, NM1, 0 );
					Quads[3].Quad[1].SetXYUV( Bx, By, 0, 0 );
					Quads[3].Quad[2].SetXYUV( Ex, Ey, NM1, NM1 );
					Quads[3].Quad[3] = Quads[3].Quad[2];
					Quads[3].Quad[4].SetXYUV( Bx, Ey, 0, NM1 );
					Quads[3].Quad[5] = Quads[3].Quad[1];

					DXUTGetD3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET , D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

					DXUTGetD3DDevice()->BeginScene();					
					DXUTGetD3DDevice()->SetVertexDeclaration( m_pUpdateDeclaration );
					DXUTGetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 4 * 2, &Quads[0].Quad[0], sizeof(RTT) );
					DXUTGetD3DDevice()->EndScene();

					pSurface->Release();
					m_pUpdate->EndPass();
				}
			}
			m_pUpdate->End();

			// Normal Creation
			if (FAILED(m_pComputeNormals->FindNextValidTechnique(NULL,&hTech)))
			{
				exit(-1);
			}

			if (SUCCEEDED(m_pComputeNormals->SetTechnique(hTech))) {

				unsigned int cPasses = 0;
				if (SUCCEEDED(m_pComputeNormals->Begin(&cPasses, 0))) {

					m_pComputeNormals->SetTexture("tElevation", Levels[l].ElevationMap);
					m_pComputeNormals->SetTexture("tCoarserNormal", Levels[min(l+1,L-1)].NormalMap);

					float TexOffset = 0.0f;

					Levels[l].NormalMap->GetSurfaceLevel(0,&pSurface);
					DXUTGetD3DDevice()->SetRenderTarget(0, pSurface);

					if ( l+1 > L-1 )
					{
						m_pComputeNormals->BeginPass(1);
					}
					else
					{
						TexOffset = TexCoarserOffset[(l%2)];
						m_pComputeNormals->BeginPass(0);
					}

					noVec4 Cto = noVec4(
						TexOffset,
						TexOffset,
						0,
						0);

					noVec4 ScaleFac = noVec4 ( -0.5f * ZScaleOverFp / Levels[l].VecScale.x, -0.5f * ZScaleOverFp / Levels[l].VecScale.y, 0.0f, 0.0f ); 
					m_pComputeNormals->SetVector("CoarserTexOffset", &Cto);
					m_pComputeNormals->SetVector("ScaleFac", &ScaleFac);
					m_pComputeNormals->SetFloat("OneOverTextureSize", TexelClip);
					m_pComputeNormals->SetFloat("ZScaleOverFp", ZScaleOverFp);
					m_pComputeNormals->SetMatrix("Proj", &m_mClipmapRTT);
					m_pComputeNormals->CommitChanges();

					DXUTGetD3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET , D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

					DXUTGetD3DDevice()->BeginScene();	
					DXUTGetD3DDevice()->SetVertexDeclaration( m_pUpdateDeclaration );
					DXUTGetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 4 * 2, &Quads[0].Quad[0], sizeof(RTT) );
					DXUTGetD3DDevice()->EndScene();

					pSurface->Release();					  
					m_pComputeNormals->EndPass();
				}
			}
			m_pComputeNormals->End();

		}

		DXUTGetD3DDevice()->SetRenderTarget(0, pBackBuffer);
		pBackBuffer->Release();	
	}

	void PreRender( D3DXVECTOR3 V, D3DXMATRIX MVP )
	{
		DWORD saveFill;
		DXUTGetD3DDevice()->GetRenderState(D3DRS_FILLMODE, &saveFill);
		DXUTGetD3DDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID );
		// Update Clipmap
		Viewer = V;

		short Dx = (short)((Viewer.x - LastViewer.x) / 2);
		short Dz = (short)((Viewer.z - LastViewer.z) / 2);

		if ( Dx != 0 || Dz != 0 )
		{
			LastViewer.x += Dx * 2.0f;
			LastViewer.z += Dz * 2.0f;

			// Find Min Level
			MinLevel = 0;

			// Terrain height at camera (x,z)
			float Height = ZScale * 0.7f * /*min(1.0f,*/ Noise(float)->getTurbulence( noVec2( (Levels[0].VecScale.z + N/2) * 0.5f/8192.0f, (Levels[0].VecScale.w + N/2) * 0.5f/8192.0f ) ) /*)*/;
			while( ((V.y - Height) * 2.5f) > (N * Levels[MinLevel].VecScale.x) && MinLevel < L-1 ) MinLevel++;
			MinLevel = min( MinLevel, L - 1 );

			Update(0, Dx, Dz);
		}

		Viewer = LastViewer;
		DXUTGetD3DDevice()->SetRenderState(D3DRS_FILLMODE, saveFill );

		// Find Frustum Planes
		FindFrustumPlanes(MVP);

		// Lock instance buffer
		noVec4 * pVertices;
		if( FAILED( m_pInstanceBuffer->Lock(0, 0, (VOID**)&pVertices, 0 ) ) )
		{
			exit(-1);
		}

		pVertices += 2 * 12 * MinLevel;

		for (int l = MinLevel; l < L; l++)
		{
			// 12 block
			Levels[l].NumInstances = 0;

			pVertices = CullBlock(0, l, pVertices);
			pVertices = CullBlock(1, l, pVertices);
			pVertices = CullBlock(2, l, pVertices);
			pVertices = CullBlock(3, l, pVertices);
			pVertices = CullBlock(4, l, pVertices);
			pVertices = CullBlock(5, l, pVertices);
			pVertices = CullBlock(6, l, pVertices);
			pVertices = CullBlock(7, l, pVertices);
			pVertices = CullBlock(8, l, pVertices);
			pVertices = CullBlock(9, l, pVertices);
			pVertices = CullBlock(10, l, pVertices);
			pVertices = CullBlock(11, l, pVertices);

			pVertices += 2 * (12 - Levels[l].NumInstances);
		}

		// 4 block finest
		NumInstancesFinest = 0;
		pVertices = CullBlockFinest(0, pVertices);
		pVertices = CullBlockFinest(1, pVertices);
		pVertices = CullBlockFinest(2, pVertices);
		pVertices = CullBlockFinest(3, pVertices);

		// For LShape & Mx3
		pVertices += 2 * (4 - NumInstancesFinest);

		pVertices += 2 * MinLevel;
		for (int l = MinLevel; l < L; l++)
		{
			*(pVertices++) = Levels[l].VecScale;
			*(pVertices++) = noVec4( TexelClip, TexelClip, Levels[l].TexScale.x, Levels[l].TexScale.y );
		}

		m_pInstanceBuffer->Unlock();
	}

	void Render( IDirect3DDevice9* pd3dDevice, D3DXMATRIX MVP )
	{
		pd3dDevice->SetStreamSource(0, m_pVertexBuffer, 0,  2 * sizeof(short) );
		pd3dDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | (UINT)1));
		pd3dDevice->SetVertexDeclaration( m_pVertexDeclaration );

		D3DXHANDLE hTech;
		if (FAILED(m_pRender->FindNextValidTechnique(NULL,&hTech)))
		{
			exit(-1);
		}

		if (SUCCEEDED(m_pRender->SetTechnique(hTech))) {

			unsigned int cPasses = 0;
			if (SUCCEEDED(m_pRender->Begin(&cPasses, 0))) {
				m_pRender->SetMatrix("worldViewProj", &MVP);
				m_pRender->SetTexture("tT0",G1);
				m_pRender->SetTexture("tT1",G2);
				m_pRender->SetTexture("tT2",G3);
				m_pRender->SetFloat("ZScaleOverFp", ZScaleOverFp );
				m_pRender->SetFloat("OneOverFp", OneOverFp );

				pd3dDevice->SetIndices(m_pIndexBufferMxM);

				m_pRender->BeginPass(0);
				for (int l = MinLevel; l < L; l++) 
				{
					// Morphing value
					const float po2 = powf(2.0f, (float)l);
					const float W = N / 10.0f;
					const float Alpha = ( ((N - 1.0f) * 0.5f  - W - 1.0f) );

					noVec4 v4TempViewer = noVec4(Viewer.x, Viewer.z, po2 * Alpha, po2 * W);
					m_pRender->SetVector("Viewer", &v4TempViewer );		
					m_pRender->SetTexture("tElevation",Levels[l].ElevationMap);
					m_pRender->SetTexture("tNormal",Levels[l].NormalMap);
					m_pRender->CommitChanges();

					if ( l == MinLevel ) 
					{
						// 4 blocks of finest
						if ( NumInstancesFinest > 0 )
						{
							pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | NumInstancesFinest) );
							pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, L * 12 * 8 * sizeof(float),  8 * sizeof(float) );
							pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesMxMIndex, 0, PrimitiveCountMxMIndex );
						}
						// 2 L-Shape
						pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | 1) );
						pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, (L * 12 + 4 + l) * 8 * sizeof(float),  8 * sizeof(float) );

						pd3dDevice->SetIndices(m_pIndexBufferLShape[1]);
						pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesLShapeIndex[1], 0, PrimitiveCountLShapeIndex[1] );

						pd3dDevice->SetIndices(m_pIndexBufferLShape[3]);
						pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesLShapeIndex[3], 0, PrimitiveCountLShapeIndex[3] );
					}
					else
					{
						// Coarser Levels: LShape
						int ls = 3;						
						if ( !Levels[l-1].Hpos && !Levels[l-1].Vpos) ls = 1;	
						else if ( Levels[l-1].Hpos && !Levels[l-1].Vpos) ls = 0;
						else if ( !Levels[l-1].Hpos && Levels[l-1].Vpos) ls = 2;

						pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | 1) );
						pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, (L * 12 + 4 + l) * 8 * sizeof(float),  8 * sizeof(float) );

						pd3dDevice->SetIndices(m_pIndexBufferLShape[ls]);
						pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesLShapeIndex[ls], 0, PrimitiveCountLShapeIndex[ls] );
					}

					// All Levels : 12 blocks
					pd3dDevice->SetIndices(m_pIndexBufferMxM);
					if ( Levels[l].NumInstances > 0 )
					{
						pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | Levels[l].NumInstances) );
						pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, l * 12 * 8 * sizeof(float),  8 * sizeof(float) );
						pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesMxMIndex, 0, PrimitiveCountMxMIndex );
					}

					// Mx3 Block 
					pd3dDevice->SetIndices(m_pIndexBufferMx3);
					pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | 1) );
					pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, (L * 12 + 4 + l) * 8 * sizeof(float),  8 * sizeof(float) );
					pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, NumVerticesMx3Index, 0, PrimitiveCountMx3Index );

					// Outer degenerate triangles
					pd3dDevice->SetIndices(m_pIndexBufferOuterDegenerateTriangles);
					pd3dDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | 1) );
					pd3dDevice->SetStreamSource(1, m_pInstanceBuffer, (L * 12 + 4 + l) * 8 * sizeof(float),  8 * sizeof(float) );
					pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumVerticesOuterDegenerateTrianglesIndex, 0, PrimitiveCountOuterDegenerateTrianglesIndex );

					pd3dDevice->SetIndices(m_pIndexBufferMxM);
				}
				m_pRender->EndPass();
			}
			m_pRender->End();
		}

		pd3dDevice->SetStreamSourceFreq(0,1);
		pd3dDevice->SetStreamSourceFreq(1,1);
	}

	~GPUGeometryClipmap()
	{
		if ( Levels != NULL)
		{
			Destroy();
		}

		SAFE_RELEASE( m_pVertexDeclaration );
		SAFE_RELEASE( m_pUpdateDeclaration );
		SAFE_RELEASE( m_pRender );
		SAFE_RELEASE( m_pUpdate );
		SAFE_RELEASE( m_pComputeNormals );
	}
