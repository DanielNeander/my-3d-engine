#include "stdafx.h"
#include "SDKMeshLoader.h"
#include "Dx11Util.h"
#include "GameApp.h"

template<typename TYPE> 
static BOOL IsErrorResource( TYPE data )
{
	if( ( TYPE )ERROR_RESOURCE_VALUE == data )
		return TRUE;
	return FALSE;
}

//--------------------------------------------------
void CDXUTSDKMesh::LoadMaterials( ID3D11Device* pd3dDevice, SDKMESH_MATERIAL* pMaterials, UINT numMaterials,
                                  SDKMESH_CALLBACKS11* pLoaderCallbacks )
{
    char strPath[MAX_PATH];

    if( pLoaderCallbacks && pLoaderCallbacks->pCreateTextureFromFile )
    {
        for( UINT m = 0; m < numMaterials; m++ )
        {
            pMaterials[m].pDiffuseTexture11 = NULL;
            pMaterials[m].pNormalTexture11 = NULL;
            pMaterials[m].pSpecularTexture11 = NULL;
            pMaterials[m].pDiffuseRV11 = NULL;
            pMaterials[m].pNormalRV11 = NULL;
            pMaterials[m].pSpecularRV11 = NULL;

            // load textures
            if( pMaterials[m].DiffuseTexture[0] != 0 )
            {
                pLoaderCallbacks->pCreateTextureFromFile( pd3dDevice,
                                                          pMaterials[m].DiffuseTexture, &pMaterials[m].pDiffuseRV11,
                                                          pLoaderCallbacks->pContext );
            }
            if( pMaterials[m].NormalTexture[0] != 0 )
            {
                pLoaderCallbacks->pCreateTextureFromFile( pd3dDevice,
                                                          pMaterials[m].NormalTexture, &pMaterials[m].pNormalRV11,
                                                          pLoaderCallbacks->pContext );
            }
            if( pMaterials[m].SpecularTexture[0] != 0 )
            {
                pLoaderCallbacks->pCreateTextureFromFile( pd3dDevice,
                                                          pMaterials[m].SpecularTexture, &pMaterials[m].pSpecularRV11,
                                                          pLoaderCallbacks->pContext );
            }
        }
    }
    else
    {
        for( UINT m = 0; m < numMaterials; m++ )
        {
            pMaterials[m].pDiffuseTexture11 = NULL;
            pMaterials[m].pNormalTexture11 = NULL;
            pMaterials[m].pSpecularTexture11 = NULL;
            pMaterials[m].pDiffuseRV11 = NULL;
            pMaterials[m].pNormalRV11 = NULL;
            pMaterials[m].pSpecularRV11 = NULL;

            // load textures
            if( pMaterials[m].DiffuseTexture[0] != 0 )
            {
                sprintf_s( strPath, MAX_PATH, "%s%s", m_strPath, pMaterials[m].DiffuseTexture );
                if( FAILED(CreateTextureFromFile( pd3dDevice, D3D11Context(),
                                                                                strPath, &pMaterials[m].pDiffuseRV11,
                                                                                true ) ) )
                    pMaterials[m].pDiffuseRV11 = ( ID3D11ShaderResourceView* )ERROR_RESOURCE_VALUE;

            }
            if( pMaterials[m].NormalTexture[0] != 0 )
            {
                sprintf_s( strPath, MAX_PATH, "%s%s", m_strPath, pMaterials[m].NormalTexture );
                if( FAILED( CreateTextureFromFile( pd3dDevice, D3D11Context(),
                                                                                strPath,
                                                                                &pMaterials[m].pNormalRV11 ) ) )
                    pMaterials[m].pNormalRV11 = ( ID3D11ShaderResourceView* )ERROR_RESOURCE_VALUE;
            }
            if( pMaterials[m].SpecularTexture[0] != 0 )
            {
                sprintf_s( strPath, MAX_PATH, "%s%s", m_strPath, pMaterials[m].SpecularTexture );
                if( FAILED( CreateTextureFromFile( pd3dDevice, D3D11Context(),
                                                                                strPath,
                                                                                &pMaterials[m].pSpecularRV11 ) ) )
                    pMaterials[m].pSpecularRV11 = ( ID3D11ShaderResourceView* )ERROR_RESOURCE_VALUE;
            }
        }
    }
}

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::CreateVertexBuffer( ID3D11Device* pd3dDevice, SDKMESH_VERTEX_BUFFER_HEADER* pHeader,
                                          void* pVertices, SDKMESH_CALLBACKS11* pLoaderCallbacks )
{
    HRESULT hr = S_OK;
    pHeader->DataOffset = 0;
    //Vertex Buffer
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = ( UINT )( pHeader->SizeBytes );
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    if( pLoaderCallbacks && pLoaderCallbacks->pCreateVertexBuffer )
    {
        pLoaderCallbacks->pCreateVertexBuffer( pd3dDevice, &pHeader->pVB11, bufferDesc, pVertices,
                                               pLoaderCallbacks->pContext );
    }
    else
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pVertices;
        hr = pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &pHeader->pVB11 );        
    }

    return hr;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::CreateIndexBuffer( ID3D11Device* pd3dDevice, SDKMESH_INDEX_BUFFER_HEADER* pHeader,
                                         void* pIndices, SDKMESH_CALLBACKS11* pLoaderCallbacks )
{
    HRESULT hr = S_OK;
    pHeader->DataOffset = 0;
    //Index Buffer
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = ( UINT )( pHeader->SizeBytes );
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    if( pLoaderCallbacks && pLoaderCallbacks->pCreateIndexBuffer )
    {
        pLoaderCallbacks->pCreateIndexBuffer( pd3dDevice, &pHeader->pIB11, bufferDesc, pIndices,
                                              pLoaderCallbacks->pContext );
    }
    else
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pIndices;
        hr = pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &pHeader->pIB11 );        
    }

    return hr;
}


//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::CreateFromFile( ID3D11Device* pDev11,                                    
                                      LPCWSTR szFileName,
                                      bool bCreateAdjacencyIndices,
                                      SDKMESH_CALLBACKS11* pLoaderCallbacks11)
{
    HRESULT hr = S_OK;

    // Find the path for the file
    FindMediaFileCch( m_strPathW, sizeof( m_strPathW ) / sizeof( WCHAR ), szFileName );
    // Open the file
    m_hFile = CreateFileW( m_strPathW, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL );
    if( INVALID_HANDLE_VALUE == m_hFile )
        return E_FAIL;

    // Change the path to just the directory
    WCHAR* pLastBSlash = wcsrchr( m_strPathW, L'\\' );
    if( pLastBSlash )
        *( pLastBSlash + 1 ) = L'\0';
    else
        *m_strPathW = L'\0';

    WideCharToMultiByte( CP_ACP, 0, m_strPathW, -1, m_strPath, MAX_PATH, NULL, FALSE );

    // Get the file size
    LARGE_INTEGER FileSize;
    GetFileSizeEx( m_hFile, &FileSize );
    UINT cBytes = FileSize.LowPart;

    // Allocate memory
    m_pStaticMeshData = new BYTE[ cBytes ];
    if( !m_pStaticMeshData )
    {
        CloseHandle( m_hFile );
        return E_OUTOFMEMORY;
    }

    // Read in the file
    DWORD dwBytesRead;
    if( !ReadFile( m_hFile, m_pStaticMeshData, cBytes, &dwBytesRead, NULL ) )
        hr = E_FAIL;

    CloseHandle( m_hFile );

    if( SUCCEEDED( hr ) )
    {
        hr = CreateFromMemory( pDev11,                               
                               m_pStaticMeshData,
                               cBytes,
                               bCreateAdjacencyIndices,
                               false,
                               pLoaderCallbacks11
                               );
        if( FAILED( hr ) )
            delete []m_pStaticMeshData;
    }

    return hr;
}

HRESULT CDXUTSDKMesh::CreateFromMemory( ID3D11Device* pDev11,                                        
                                        BYTE* pData,
                                        UINT DataBytes,
                                        bool bCreateAdjacencyIndices,
                                        bool bCopyStatic,
                                        SDKMESH_CALLBACKS11* pLoaderCallbacks11
                                        )
{
    HRESULT hr = E_FAIL;
    D3DXVECTOR3 lower; 
    D3DXVECTOR3 upper;     
    
	m_pDev11 = pDev11;

    // Set outstanding resources to zero
    m_NumOutstandingResources = 0;

    if( bCopyStatic )
    {
        SDKMESH_HEADER* pHeader = ( SDKMESH_HEADER* )pData;

        SIZE_T StaticSize = ( SIZE_T )( pHeader->HeaderSize + pHeader->NonBufferDataSize );
        m_pHeapData = new BYTE[ StaticSize ];
        if( !m_pHeapData )
            return hr;

        m_pStaticMeshData = m_pHeapData;

        CopyMemory( m_pStaticMeshData, pData, StaticSize );
    }
    else
    {
        m_pHeapData = pData;
        m_pStaticMeshData = pData;
    }

    // Pointer fixup
    m_pMeshHeader = ( SDKMESH_HEADER* )m_pStaticMeshData;
    m_pVertexBufferArray = ( SDKMESH_VERTEX_BUFFER_HEADER* )( m_pStaticMeshData +
                                                              m_pMeshHeader->VertexStreamHeadersOffset );
    m_pIndexBufferArray = ( SDKMESH_INDEX_BUFFER_HEADER* )( m_pStaticMeshData +
                                                            m_pMeshHeader->IndexStreamHeadersOffset );
    m_pMeshArray = ( SDKMESH_MESH* )( m_pStaticMeshData + m_pMeshHeader->MeshDataOffset );
    m_pSubsetArray = ( SDKMESH_SUBSET* )( m_pStaticMeshData + m_pMeshHeader->SubsetDataOffset );
    m_pFrameArray = ( SDKMESH_FRAME* )( m_pStaticMeshData + m_pMeshHeader->FrameDataOffset );
    m_pMaterialArray = ( SDKMESH_MATERIAL* )( m_pStaticMeshData + m_pMeshHeader->MaterialDataOffset );

    // Setup subsets
    for( UINT i = 0; i < m_pMeshHeader->NumMeshes; i++ )
    {
        m_pMeshArray[i].pSubsets = ( UINT* )( m_pStaticMeshData + m_pMeshArray[i].SubsetOffset );
        m_pMeshArray[i].pFrameInfluences = ( UINT* )( m_pStaticMeshData + m_pMeshArray[i].FrameInfluenceOffset );
		m_pMeshArray[i].Culled = false;
    }

    // error condition
    if( m_pMeshHeader->Version != SDKMESH_FILE_VERSION )
    {
        hr = E_NOINTERFACE;
        goto LoadError;
    }

    // Setup buffer data pointer
    BYTE* pBufferData = pData + m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Get the start of the buffer data
    UINT64 BufferDataStart = m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Create VBs
    m_ppVertices = new BYTE*[m_pMeshHeader->NumVertexBuffers];
    for( UINT i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
    {
        BYTE* pVertices = NULL;
        pVertices = ( BYTE* )( pBufferData + ( m_pVertexBufferArray[i].DataOffset - BufferDataStart ) );

        if( pDev11 )
            CreateVertexBuffer( pDev11, &m_pVertexBufferArray[i], pVertices, pLoaderCallbacks11 );        

        m_ppVertices[i] = pVertices;
    }

    // Create IBs
    m_ppIndices = new BYTE*[m_pMeshHeader->NumIndexBuffers];
    for( UINT i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
    {
        BYTE* pIndices = NULL;
        pIndices = ( BYTE* )( pBufferData + ( m_pIndexBufferArray[i].DataOffset - BufferDataStart ) );

        if( pDev11 )
            CreateIndexBuffer( pDev11, &m_pIndexBufferArray[i], pIndices, pLoaderCallbacks11 );
        
        m_ppIndices[i] = pIndices;
    }

    // Load Materials
    if( pDev11 )
        LoadMaterials( pDev11, m_pMaterialArray, m_pMeshHeader->NumMaterials, pLoaderCallbacks11 );
    
    // Create a place to store our bind pose frame matrices
    m_pBindPoseFrameMatrices = new D3DXMATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pBindPoseFrameMatrices )
        goto LoadError;

    // Create a place to store our transformed frame matrices
    m_pTransformedFrameMatrices = new D3DXMATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pTransformedFrameMatrices )
        goto LoadError;
    m_pWorldPoseFrameMatrices = new D3DXMATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pWorldPoseFrameMatrices )
        goto LoadError;

    SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

    // update bounding volume 
    SDKMESH_MESH* currentMesh = &m_pMeshArray[0];
    int tris = 0;
    for (UINT meshi=0; meshi < m_pMeshHeader->NumMeshes; ++meshi) {
        lower.x = FLT_MAX; lower.y = FLT_MAX; lower.z = FLT_MAX;
        upper.x = -FLT_MAX; upper.y = -FLT_MAX; upper.z = -FLT_MAX;
        currentMesh = GetMesh( meshi );
        INT indsize;
        if (m_pIndexBufferArray[currentMesh->IndexBuffer].IndexType == IT_16BIT ) {
            indsize = 2;
        }else {
            indsize = 4;        
        }

        for( UINT subset = 0; subset < currentMesh->NumSubsets; subset++ )
        {
            pSubset = GetSubset( meshi, subset ); //&m_pSubsetArray[ currentMesh->pSubsets[subset] ];

            PrimType = GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
            assert( PrimType == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );// only triangle lists are handled.

            UINT IndexCount = ( UINT )pSubset->IndexCount;
            UINT IndexStart = ( UINT )pSubset->IndexStart;

            /*if( bAdjacent )
            {
                IndexCount *= 2;
                IndexStart *= 2;
            }*/
     
        //BYTE* pIndices = NULL;
            //m_ppIndices[i]
            UINT *ind = ( UINT * )m_ppIndices[currentMesh->IndexBuffer];
            FLOAT *verts =  ( FLOAT* )m_ppVertices[currentMesh->VertexBuffers[0]];
            UINT stride = (UINT)m_pVertexBufferArray[currentMesh->VertexBuffers[0]].StrideBytes;
            assert (stride % 4 == 0);
            stride /=4;
            for (UINT vertind = IndexStart; vertind < IndexStart + IndexCount; ++vertind) {
                UINT current_ind=0;
                if (indsize == 2) {
                    UINT ind_div2 = vertind / 2;
                    current_ind = ind[ind_div2];
                    if (vertind %2 ==0) {
                        current_ind = current_ind << 16;
                        current_ind = current_ind >> 16;
                    }else {
                        current_ind = current_ind >> 16;
                    }
                }else {
                    current_ind = ind[vertind];
                }
                tris++;
                D3DXVECTOR3 *pt = (D3DXVECTOR3*)&(verts[stride * current_ind]);
                if (pt->x < lower.x) {
                    lower.x = pt->x;
                }
                if (pt->y < lower.y) {
                    lower.y = pt->y;
                }
                if (pt->z < lower.z) {
                    lower.z = pt->z;
                }
                if (pt->x > upper.x) {
                    upper.x = pt->x;
                }
                if (pt->y > upper.y) {
                    upper.y = pt->y;
                }
                if (pt->z > upper.z) {
                    upper.z = pt->z;
                }
                //BYTE** m_ppVertices;
                //BYTE** m_ppIndices;
            }
            //pd3dDeviceContext->DrawIndexed( IndexCount, IndexStart, VertexStart );
        }

        D3DXVECTOR3 half = upper - lower;
        half *=0.5f;

        currentMesh->BoundingBoxCenter = lower + half;
        currentMesh->BoundingBoxExtents = half;

    }
    // Update 
   
LoadError :

    hr = S_OK;

    return hr;
}

//--------------------------------------------------------------------------------------
// transform bind pose frame using a recursive traversal
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::TransformBindPoseFrame( UINT iFrame, D3DXMATRIX* pParentWorld )
{
    if( !m_pBindPoseFrameMatrices )
        return;

    // Transform ourselves
    D3DXMATRIX LocalWorld;
    D3DXMatrixMultiply( &LocalWorld, &m_pFrameArray[iFrame].Matrix, pParentWorld );
    m_pBindPoseFrameMatrices[iFrame] = LocalWorld;

    // Transform our siblings
    if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
        TransformBindPoseFrame( m_pFrameArray[iFrame].SiblingFrame, pParentWorld );

    // Transform our children
    if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
        TransformBindPoseFrame( m_pFrameArray[iFrame].ChildFrame, &LocalWorld );
}

//--------------------------------------------------------------------------------------
// transform frame using a recursive traversal
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::TransformFrame( UINT iFrame, D3DXMATRIX* pParentWorld, double fTime )
{
    // Get the tick data
    D3DXMATRIX LocalTransform;
    UINT iTick = GetAnimationKeyFromTime( fTime );

    if( INVALID_ANIMATION_DATA != m_pFrameArray[iFrame].AnimationDataIndex )
    {
        SDKANIMATION_FRAME_DATA* pFrameData = &m_pAnimationFrameData[ m_pFrameArray[iFrame].AnimationDataIndex ];
        SDKANIMATION_DATA* pData = &pFrameData->pAnimationData[ iTick ];

        // turn it into a matrix (Ignore scaling for now)
        D3DXVECTOR3 parentPos = pData->Translation;
        D3DXMATRIX mTranslate;
        D3DXMatrixTranslation( &mTranslate, parentPos.x, parentPos.y, parentPos.z );

        D3DXQUATERNION quat;
        D3DXMATRIX mQuat;
        quat.w = pData->Orientation.w;
        quat.x = pData->Orientation.x;
        quat.y = pData->Orientation.y;
        quat.z = pData->Orientation.z;
        if( quat.w == 0 && quat.x == 0 && quat.y == 0 && quat.z == 0 )
            D3DXQuaternionIdentity( &quat );
        D3DXQuaternionNormalize( &quat, &quat );
        D3DXMatrixRotationQuaternion( &mQuat, &quat );
        LocalTransform = ( mQuat * mTranslate );
    }
    else
    {
        LocalTransform = m_pFrameArray[iFrame].Matrix;
    }

    // Transform ourselves
    D3DXMATRIX LocalWorld;
    D3DXMatrixMultiply( &LocalWorld, &LocalTransform, pParentWorld );
    m_pTransformedFrameMatrices[iFrame] = LocalWorld;
    m_pWorldPoseFrameMatrices[iFrame] = LocalWorld;

    // Transform our siblings
    if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
        TransformFrame( m_pFrameArray[iFrame].SiblingFrame, pParentWorld, fTime );

    // Transform our children
    if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
        TransformFrame( m_pFrameArray[iFrame].ChildFrame, &LocalWorld, fTime );
}

//--------------------------------------------------------------------------------------
// transform frame assuming that it is an absolute transformation
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::TransformFrameAbsolute( UINT iFrame, double fTime )
{
    D3DXMATRIX mTrans1;
    D3DXMATRIX mTrans2;
    D3DXMATRIX mRot1;
    D3DXMATRIX mRot2;
    D3DXQUATERNION quat1;
    D3DXQUATERNION quat2;
    D3DXMATRIX mTo;
    D3DXMATRIX mInvTo;
    D3DXMATRIX mFrom;

    UINT iTick = GetAnimationKeyFromTime( fTime );

    if( INVALID_ANIMATION_DATA != m_pFrameArray[iFrame].AnimationDataIndex )
    {
        SDKANIMATION_FRAME_DATA* pFrameData = &m_pAnimationFrameData[ m_pFrameArray[iFrame].AnimationDataIndex ];
        SDKANIMATION_DATA* pData = &pFrameData->pAnimationData[ iTick ];
        SDKANIMATION_DATA* pDataOrig = &pFrameData->pAnimationData[ 0 ];

        D3DXMatrixTranslation( &mTrans1, -pDataOrig->Translation.x,
                               -pDataOrig->Translation.y,
                               -pDataOrig->Translation.z );
        D3DXMatrixTranslation( &mTrans2, pData->Translation.x,
                               pData->Translation.y,
                               pData->Translation.z );

        quat1.x = pDataOrig->Orientation.x;
        quat1.y = pDataOrig->Orientation.y;
        quat1.z = pDataOrig->Orientation.z;
        quat1.w = pDataOrig->Orientation.w;
        D3DXQuaternionInverse( &quat1, &quat1 );
        D3DXMatrixRotationQuaternion( &mRot1, &quat1 );
        mInvTo = mTrans1 * mRot1;

        quat2.x = pData->Orientation.x;
        quat2.y = pData->Orientation.y;
        quat2.z = pData->Orientation.z;
        quat2.w = pData->Orientation.w;
        D3DXMatrixRotationQuaternion( &mRot2, &quat2 );
        mFrom = mRot2 * mTrans2;

        D3DXMATRIX mOutput = mInvTo * mFrom;
        m_pTransformedFrameMatrices[iFrame] = mOutput;
    }
}

#define MAX_D3D11_VERTEX_STREAMS D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::RenderMesh( UINT iMesh,
                               bool bAdjacent,
                               ID3D11DeviceContext* pd3dDeviceContext,
                               UINT iDiffuseSlot,
                               UINT iNormalSlot,
                               UINT iSpecularSlot )
{
    if( 0 < GetOutstandingBufferResources() )
        return;
	
    SDKMESH_MESH* pMesh = &m_pMeshArray[iMesh];

	if (pMesh->Culled) return;

    UINT Strides[MAX_D3D11_VERTEX_STREAMS];
    UINT Offsets[MAX_D3D11_VERTEX_STREAMS];
    ID3D11Buffer* pVB[MAX_D3D11_VERTEX_STREAMS];

    if( pMesh->NumVertexBuffers > MAX_D3D11_VERTEX_STREAMS )
        return;

    for( UINT64 i = 0; i < pMesh->NumVertexBuffers; i++ )
    {
        pVB[i] = m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].pVB11;
        Strides[i] = ( UINT )m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].StrideBytes;
        Offsets[i] = 0;
    }

    SDKMESH_INDEX_BUFFER_HEADER* pIndexBufferArray;
    if( bAdjacent )
        pIndexBufferArray = m_pAdjacencyIndexBufferArray;
    else
        pIndexBufferArray = m_pIndexBufferArray;

    ID3D11Buffer* pIB = pIndexBufferArray[ pMesh->IndexBuffer ].pIB11;
    DXGI_FORMAT ibFormat = DXGI_FORMAT_R16_UINT;
    switch( pIndexBufferArray[ pMesh->IndexBuffer ].IndexType )
    {
    case IT_16BIT:
        ibFormat = DXGI_FORMAT_R16_UINT;
        break;
    case IT_32BIT:
        ibFormat = DXGI_FORMAT_R32_UINT;
        break;
    };

    pd3dDeviceContext->IASetVertexBuffers( 0, pMesh->NumVertexBuffers, pVB, Strides, Offsets );
    pd3dDeviceContext->IASetIndexBuffer( pIB, ibFormat, 0 );

    SDKMESH_SUBSET* pSubset = NULL;
    SDKMESH_MATERIAL* pMat = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

    for( UINT subset = 0; subset < pMesh->NumSubsets; subset++ )
    {
        pSubset = &m_pSubsetArray[ pMesh->pSubsets[subset] ];

        PrimType = GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
        if( bAdjacent )
        {
            switch( PrimType )
            {
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
                PrimType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
                break;
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
                PrimType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
                break;
            case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
                PrimType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
                break;
            case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
                PrimType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
                break;
            }
        }

        pd3dDeviceContext->IASetPrimitiveTopology( PrimType );

        pMat = &m_pMaterialArray[ pSubset->MaterialID ];
        if( iDiffuseSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pDiffuseRV11 ) )
            pd3dDeviceContext->PSSetShaderResources( iDiffuseSlot, 1, &pMat->pDiffuseRV11 );
        if( iNormalSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pNormalRV11 ) )
            pd3dDeviceContext->PSSetShaderResources( iNormalSlot, 1, &pMat->pNormalRV11 );
        if( iSpecularSlot != INVALID_SAMPLER_SLOT && !IsErrorResource( pMat->pSpecularRV11 ) )
            pd3dDeviceContext->PSSetShaderResources( iSpecularSlot, 1, &pMat->pSpecularRV11 );

        UINT IndexCount = ( UINT )pSubset->IndexCount;
        UINT IndexStart = ( UINT )pSubset->IndexStart;
        UINT VertexStart = ( UINT )pSubset->VertexStart;
        if( bAdjacent )
        {
            IndexCount *= 2;
            IndexStart *= 2;
        }

        pd3dDeviceContext->DrawIndexed( IndexCount, IndexStart, VertexStart );
    }
}

//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::RenderFrame( UINT iFrame,
                                bool bAdjacent,
                                ID3D11DeviceContext* pd3dDeviceContext,
                                UINT iDiffuseSlot,
                                UINT iNormalSlot,
                                UINT iSpecularSlot )
{
    if( !m_pStaticMeshData || !m_pFrameArray )
        return;

    if( m_pFrameArray[iFrame].Mesh != INVALID_MESH )
    {
        RenderMesh( m_pFrameArray[iFrame].Mesh,
                    bAdjacent,
                    pd3dDeviceContext,
                    iDiffuseSlot,
                    iNormalSlot,
                    iSpecularSlot );
    }

    // Render our children
    if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
        RenderFrame( m_pFrameArray[iFrame].ChildFrame, bAdjacent, pd3dDeviceContext, iDiffuseSlot, 
                     iNormalSlot, iSpecularSlot );

    // Render our siblings
    if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
        RenderFrame( m_pFrameArray[iFrame].SiblingFrame, bAdjacent, pd3dDeviceContext, iDiffuseSlot, 
                     iNormalSlot, iSpecularSlot );
}

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
CDXUTSDKMesh::CDXUTSDKMesh() : m_NumOutstandingResources( 0 ),
                               m_bLoading( false ),
                               m_hFile( 0 ),
                               m_hFileMappingObject( 0 ),
                               m_pMeshHeader( NULL ),
                               m_pStaticMeshData( NULL ),
                               m_pHeapData( NULL ),
                               m_pAdjacencyIndexBufferArray( NULL ),
                               m_pAnimationData( NULL ),
                               m_pAnimationHeader( NULL ),
                               m_ppVertices( NULL ),
                               m_ppIndices( NULL ),
                               m_pBindPoseFrameMatrices( NULL ),
                               m_pTransformedFrameMatrices( NULL ),
                               m_pWorldPoseFrameMatrices( NULL ),                               
							   m_pDev11( NULL )
{
}


//--------------------------------------------------------------------------------------
CDXUTSDKMesh::~CDXUTSDKMesh()
{
    Destroy();
}

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::Create( ID3D11Device* pDev11, LPCWSTR szFileName, bool bCreateAdjacencyIndices,
                              SDKMESH_CALLBACKS11* pLoaderCallbacks )
{
    return CreateFromFile( pDev11,  szFileName, bCreateAdjacencyIndices, pLoaderCallbacks );
}

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::Create( ID3D11Device* pDev11, BYTE* pData, UINT DataBytes, bool bCreateAdjacencyIndices,
                              bool bCopyStatic, SDKMESH_CALLBACKS11* pLoaderCallbacks )
{
    return CreateFromMemory( pDev11, pData, DataBytes, bCreateAdjacencyIndices, bCopyStatic,
                             pLoaderCallbacks );
}

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::LoadAnimation( WCHAR* szFileName )
{
    HRESULT hr = E_FAIL;
    DWORD dwBytesRead = 0;
    LARGE_INTEGER liMove;
    WCHAR strPath[MAX_PATH];

    // Find the path for the file
     FindMediaFileCch( strPath, MAX_PATH, szFileName );

    // Open the file
    HANDLE hFile = CreateFileW( strPath, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if( INVALID_HANDLE_VALUE == hFile )
        return E_FAIL;

    /////////////////////////
    // Header
    SDKANIMATION_FILE_HEADER fileheader;
    if( !ReadFile( hFile, &fileheader, sizeof( SDKANIMATION_FILE_HEADER ), &dwBytesRead, NULL ) )
        goto LoadError;

    //allocate
    m_pAnimationData = new BYTE[ ( size_t )( sizeof( SDKANIMATION_FILE_HEADER ) + fileheader.AnimationDataSize ) ];
    if( !m_pAnimationData )
    {
        hr = E_OUTOFMEMORY;
        goto LoadError;
    }

    // read it all in
    liMove.QuadPart = 0;
    if( !SetFilePointerEx( hFile, liMove, NULL, FILE_BEGIN ) )
        goto LoadError;
    if( !ReadFile( hFile, m_pAnimationData, ( DWORD )( sizeof( SDKANIMATION_FILE_HEADER ) +
                                                       fileheader.AnimationDataSize ), &dwBytesRead, NULL ) )
        goto LoadError;

    // pointer fixup
    m_pAnimationHeader = ( SDKANIMATION_FILE_HEADER* )m_pAnimationData;
    m_pAnimationFrameData = ( SDKANIMATION_FRAME_DATA* )( m_pAnimationData + m_pAnimationHeader->AnimationDataOffset );

    UINT64 BaseOffset = sizeof( SDKANIMATION_FILE_HEADER );
    for( UINT i = 0; i < m_pAnimationHeader->NumFrames; i++ )
    {
        m_pAnimationFrameData[i].pAnimationData = ( SDKANIMATION_DATA* )( m_pAnimationData +
                                                                          m_pAnimationFrameData[i].DataOffset +
                                                                          BaseOffset );
        SDKMESH_FRAME* pFrame = FindFrame( m_pAnimationFrameData[i].FrameName );
        if( pFrame )
        {
            pFrame->AnimationDataIndex = i;
        }
    }

    hr = S_OK;
LoadError:
    CloseHandle( hFile );
    return hr;
}

//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::Destroy()
{
    if( !CheckLoadDone() )
        return;

    if( m_pStaticMeshData )
    {
        if( m_pMaterialArray )
        {
            for( UINT64 m = 0; m < m_pMeshHeader->NumMaterials; m++ )
            {
               /* if( m_pDev9 )
                {
                    if( !IsErrorResource( m_pMaterialArray[m].pDiffuseTexture9 ) )
                        SAFE_RELEASE( m_pMaterialArray[m].pDiffuseTexture9 );
                    if( !IsErrorResource( m_pMaterialArray[m].pNormalTexture9 ) )
                        SAFE_RELEASE( m_pMaterialArray[m].pNormalTexture9 );
                    if( !IsErrorResource( m_pMaterialArray[m].pSpecularTexture9 ) )
                        SAFE_RELEASE( m_pMaterialArray[m].pSpecularTexture9 );
                }
				else*/ if( m_pDev11 )
                {
                    //ID3D11Resource* pRes = NULL;
                    if( m_pMaterialArray[m].pDiffuseRV11 && !IsErrorResource( m_pMaterialArray[m].pDiffuseRV11 ) )
                    {
                        //m_pMaterialArray[m].pDiffuseRV11->GetResource( &pRes );
                        //SAFE_RELEASE( pRes );

                        SAFE_RELEASE( m_pMaterialArray[m].pDiffuseRV11 );
                    }
                    if( m_pMaterialArray[m].pNormalRV11 && !IsErrorResource( m_pMaterialArray[m].pNormalRV11 ) )
                    {
                        //m_pMaterialArray[m].pNormalRV11->GetResource( &pRes );
                        //SAFE_RELEASE( pRes );

                        SAFE_RELEASE( m_pMaterialArray[m].pNormalRV11 );
                    }
                    if( m_pMaterialArray[m].pSpecularRV11 && !IsErrorResource( m_pMaterialArray[m].pSpecularRV11 ) )
                    {
                        //m_pMaterialArray[m].pSpecularRV11->GetResource( &pRes );
                        //SAFE_RELEASE( pRes );

                        SAFE_RELEASE( m_pMaterialArray[m].pSpecularRV11 );
                    }
                }
            }
        }

        //for( UINT64 i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
        //{
        //    if( !IsErrorResource( m_pVertexBufferArray[i].pVB9 ) )
        //        SAFE_RELEASE( m_pVertexBufferArray[i].pVB9 );
        //}

        //for( UINT64 i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
        //{
        //    if( !IsErrorResource( m_pIndexBufferArray[i].pIB9 ) )
        //        SAFE_RELEASE( m_pIndexBufferArray[i].pIB9 );
        //}
    }

    if( m_pAdjacencyIndexBufferArray )
    {
        for( UINT64 i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
        {
            SAFE_RELEASE( m_pAdjacencyIndexBufferArray[i].pIB11 );
        }
    }
    SAFE_DELETE_ARRAY( m_pAdjacencyIndexBufferArray );

    SAFE_DELETE_ARRAY( m_pHeapData );
    m_pStaticMeshData = NULL;
    SAFE_DELETE_ARRAY( m_pAnimationData );
    SAFE_DELETE_ARRAY( m_pBindPoseFrameMatrices );
    SAFE_DELETE_ARRAY( m_pTransformedFrameMatrices );
    SAFE_DELETE_ARRAY( m_pWorldPoseFrameMatrices );

    SAFE_DELETE_ARRAY( m_ppVertices );
    SAFE_DELETE_ARRAY( m_ppIndices );

    m_pMeshHeader = NULL;
    m_pVertexBufferArray = NULL;
    m_pIndexBufferArray = NULL;
    m_pMeshArray = NULL;
    m_pSubsetArray = NULL;
    m_pFrameArray = NULL;
    m_pMaterialArray = NULL;

    m_pAnimationHeader = NULL;
    m_pAnimationFrameData = NULL;

}

//--------------------------------------------------------------------------------------
// transform the bind pose
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::TransformBindPose( D3DXMATRIX* pWorld )
{
    TransformBindPoseFrame( 0, pWorld );
}

//--------------------------------------------------------------------------------------
// transform the mesh frames according to the animation for time fTime
//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::TransformMesh( D3DXMATRIX* pWorld, double fTime )
{
    if( m_pAnimationHeader == NULL || FTT_RELATIVE == m_pAnimationHeader->FrameTransformType )
    {
        TransformFrame( 0, pWorld, fTime );

        // For each frame, move the transform to the bind pose, then
        // move it to the final position
        D3DXMATRIX mInvBindPose;
        D3DXMATRIX mFinal;
        for( UINT i = 0; i < m_pMeshHeader->NumFrames; i++ )
        {
            D3DXMatrixInverse( &mInvBindPose, NULL, &m_pBindPoseFrameMatrices[i] );
            mFinal = mInvBindPose * m_pTransformedFrameMatrices[i];
            m_pTransformedFrameMatrices[i] = mFinal;
        }
    }
    else if( FTT_ABSOLUTE == m_pAnimationHeader->FrameTransformType )
    {
        for( UINT i = 0; i < m_pAnimationHeader->NumFrames; i++ )
            TransformFrameAbsolute( i, fTime );
    }
}


//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::Render( ID3D11DeviceContext* pd3dDeviceContext,
                           UINT iDiffuseSlot,
                           UINT iNormalSlot,
                           UINT iSpecularSlot )
{
    RenderFrame( 0, false, pd3dDeviceContext, iDiffuseSlot, iNormalSlot, iSpecularSlot );
}

//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::RenderAdjacent( ID3D11DeviceContext* pd3dDeviceContext,
                                   UINT iDiffuseSlot,
                                   UINT iNormalSlot,
                                   UINT iSpecularSlot )
{
    RenderFrame( 0, true, pd3dDeviceContext, iDiffuseSlot, iNormalSlot, iSpecularSlot );
}



//--------------------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY CDXUTSDKMesh::GetPrimitiveType11( SDKMESH_PRIMITIVE_TYPE PrimType )
{
    D3D11_PRIMITIVE_TOPOLOGY retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch( PrimType )
    {
        case PT_TRIANGLE_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case PT_TRIANGLE_STRIP:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case PT_LINE_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case PT_LINE_STRIP:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case PT_POINT_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case PT_TRIANGLE_LIST_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            break;
        case PT_TRIANGLE_STRIP_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            break;
        case PT_LINE_LIST_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            break;
        case PT_LINE_STRIP_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            break;
    };

    return retType;
}

//--------------------------------------------------------------------------------------
DXGI_FORMAT CDXUTSDKMesh::GetIBFormat11( UINT iMesh )
{
    switch( m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].IndexType )
    {
        case IT_16BIT:
            return DXGI_FORMAT_R16_UINT;
        case IT_32BIT:
            return DXGI_FORMAT_R32_UINT;
    };
    return DXGI_FORMAT_R16_UINT;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetVB11( UINT iMesh, UINT iVB )
{
    return m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].pVB11;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetIB11( UINT iMesh )
{
    return m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11;
}
SDKMESH_INDEX_TYPE CDXUTSDKMesh::GetIndexType( UINT iMesh ) 
{
    return ( SDKMESH_INDEX_TYPE ) m_pIndexBufferArray[m_pMeshArray[ iMesh ].IndexBuffer].IndexType;
}
//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetAdjIB11( UINT iMesh )
{
    return m_pAdjacencyIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11;
}

//--------------------------------------------------------------------------------------
//D3DPRIMITIVETYPE CDXUTSDKMesh::GetPrimitiveType9( SDKMESH_PRIMITIVE_TYPE PrimType )
//{
//    D3DPRIMITIVETYPE retType = D3DPT_TRIANGLELIST;
//
//    switch( PrimType )
//    {
//        case PT_TRIANGLE_LIST:
//            retType = D3DPT_TRIANGLELIST;
//            break;
//        case PT_TRIANGLE_STRIP:
//            retType = D3DPT_TRIANGLESTRIP;
//            break;
//        case PT_LINE_LIST:
//            retType = D3DPT_LINELIST;
//            break;
//        case PT_LINE_STRIP:
//            retType = D3DPT_LINESTRIP;
//            break;
//        case PT_POINT_LIST:
//            retType = D3DPT_POINTLIST;
//            break;
//    };
//
//    return retType;
//}

//--------------------------------------------------------------------------------------
char* CDXUTSDKMesh::GetMeshPathA()
{
    return m_strPath;
}

//--------------------------------------------------------------------------------------
WCHAR* CDXUTSDKMesh::GetMeshPathW()
{
    return m_strPathW;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumMeshes()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMeshes;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumMaterials()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMaterials;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumVBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumVertexBuffers;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumIBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumIndexBuffers;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetVB11At( UINT iVB )
{
    return m_pVertexBufferArray[ iVB ].pVB11;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetIB11At( UINT iIB )
{
    return m_pIndexBufferArray[ iIB ].pIB11;
}

//--------------------------------------------------------------------------------------
//IDirect3DVertexBuffer9* CDXUTSDKMesh::GetVB9At( UINT iVB )
//{
//    return m_pVertexBufferArray[ iVB ].pVB9;
//}
//
////--------------------------------------------------------------------------------------
//IDirect3DIndexBuffer9* CDXUTSDKMesh::GetIB9At( UINT iIB )
//{
//    return m_pIndexBufferArray[ iIB ].pIB9;
//}

//--------------------------------------------------------------------------------------
BYTE* CDXUTSDKMesh::GetRawVerticesAt( UINT iVB )
{
    return m_ppVertices[iVB];
}

//--------------------------------------------------------------------------------------
BYTE* CDXUTSDKMesh::GetRawIndicesAt( UINT iIB )
{
    return m_ppIndices[iIB];
}

//--------------------------------------------------------------------------------------
SDKMESH_MATERIAL* CDXUTSDKMesh::GetMaterial( UINT iMaterial )
{
    return &m_pMaterialArray[ iMaterial ];
}

//--------------------------------------------------------------------------------------
SDKMESH_MESH* CDXUTSDKMesh::GetMesh( UINT iMesh )
{
    return &m_pMeshArray[ iMesh ];
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumSubsets( UINT iMesh )
{
    return m_pMeshArray[ iMesh ].NumSubsets;
}

//--------------------------------------------------------------------------------------
SDKMESH_SUBSET* CDXUTSDKMesh::GetSubset( UINT iMesh, UINT iSubset )
{
    return &m_pSubsetArray[ m_pMeshArray[ iMesh ].pSubsets[iSubset] ];
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetVertexStride( UINT iMesh, UINT iVB )
{
    return ( UINT )m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].StrideBytes;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumFrames()
{
    return m_pMeshHeader->NumFrames;
}

//--------------------------------------------------------------------------------------
SDKMESH_FRAME* CDXUTSDKMesh::GetFrame( UINT iFrame )
{
    assert( iFrame < m_pMeshHeader->NumFrames );
    return &m_pFrameArray[ iFrame ];
}

//--------------------------------------------------------------------------------------
SDKMESH_FRAME* CDXUTSDKMesh::FindFrame( char* pszName )
{
    for( UINT i = 0; i < m_pMeshHeader->NumFrames; i++ )
    {
        if( _stricmp( m_pFrameArray[i].Name, pszName ) == 0 )
        {
            return &m_pFrameArray[i];
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------
UINT64 CDXUTSDKMesh::GetNumVertices( UINT iMesh, UINT iVB )
{
    return m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].NumVertices;
}

//--------------------------------------------------------------------------------------
UINT64 CDXUTSDKMesh::GetNumIndices( UINT iMesh )
{
    return m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].NumIndices;
}

//--------------------------------------------------------------------------------------
D3DXVECTOR3 CDXUTSDKMesh::GetMeshBBoxCenter( UINT iMesh )
{
    return m_pMeshArray[iMesh].BoundingBoxCenter;
}

//--------------------------------------------------------------------------------------
D3DXVECTOR3 CDXUTSDKMesh::GetMeshBBoxExtents( UINT iMesh )
{
    return m_pMeshArray[iMesh].BoundingBoxExtents;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetOutstandingResources()
{
    UINT outstandingResources = 0;
    if( !m_pMeshHeader )
        return 1;

    outstandingResources += GetOutstandingBufferResources();

	if( m_pDev11 )
	{
		for( UINT i = 0; i < m_pMeshHeader->NumMaterials; i++ )
        {
            if( m_pMaterialArray[i].DiffuseTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pDiffuseRV11 && !IsErrorResource( m_pMaterialArray[i].pDiffuseRV11 ) )
                    outstandingResources ++;
            }

            if( m_pMaterialArray[i].NormalTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pNormalRV11 && !IsErrorResource( m_pMaterialArray[i].pNormalRV11 ) )
                    outstandingResources ++;
            }

            if( m_pMaterialArray[i].SpecularTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pSpecularRV11 && !IsErrorResource( m_pMaterialArray[i].pSpecularRV11 ) )
                    outstandingResources ++;
            }
        }
	}
    else
    {
       /* for( UINT i = 0; i < m_pMeshHeader->NumMaterials; i++ )
        {
            if( m_pMaterialArray[i].DiffuseTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pDiffuseTexture9 && !IsErrorResource( m_pMaterialArray[i].pDiffuseTexture9 ) )
                    outstandingResources ++;
            }

            if( m_pMaterialArray[i].NormalTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pNormalTexture9 && !IsErrorResource( m_pMaterialArray[i].pNormalTexture9 ) )
                    outstandingResources ++;
            }

            if( m_pMaterialArray[i].SpecularTexture[0] != 0 )
            {
                if( !m_pMaterialArray[i].pSpecularTexture9 &&
                    !IsErrorResource( m_pMaterialArray[i].pSpecularTexture9 ) )
                    outstandingResources ++;
            }
        }*/
    }

    return outstandingResources;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetOutstandingBufferResources()
{
    UINT outstandingResources = 0;
    if( !m_pMeshHeader )
        return 1;

    //for( UINT i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
    //{
    //    if( !m_pVertexBufferArray[i].pVB9 && !IsErrorResource( m_pVertexBufferArray[i].pVB9 ) )
    //        outstandingResources ++;
    //}

    //for( UINT i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
    //{
    //    if( !m_pIndexBufferArray[i].pIB9 && !IsErrorResource( m_pIndexBufferArray[i].pIB9 ) )
    //        outstandingResources ++;
    //}

    return outstandingResources;
}

//--------------------------------------------------------------------------------------
bool CDXUTSDKMesh::CheckLoadDone()
{
    if( 0 == GetOutstandingResources() )
    {
        m_bLoading = false;
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------
bool CDXUTSDKMesh::IsLoaded()
{
    if( m_pStaticMeshData && !m_bLoading )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------
bool CDXUTSDKMesh::IsLoading()
{
    return m_bLoading;
}

//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::SetLoading( bool bLoading )
{
    m_bLoading = bLoading;
}

//--------------------------------------------------------------------------------------
BOOL CDXUTSDKMesh::HadLoadingError()
{
    if( m_pMeshHeader )
    {
        //for( UINT i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
        //{
        //    if( IsErrorResource( m_pVertexBufferArray[i].pVB9 ) )
        //        return TRUE;
        //}

        //for( UINT i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
        //{
        //    if( IsErrorResource( m_pIndexBufferArray[i].pIB9 ) )
        //        return TRUE;
        //}
    }

    return FALSE;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumInfluences( UINT iMesh )
{
    return m_pMeshArray[iMesh].NumFrameInfluences;
}

//--------------------------------------------------------------------------------------
const D3DXMATRIX* CDXUTSDKMesh::GetMeshInfluenceMatrix( UINT iMesh, UINT iInfluence )
{
    UINT iFrame = m_pMeshArray[iMesh].pFrameInfluences[ iInfluence ];
    return &m_pTransformedFrameMatrices[iFrame];
}

const D3DXMATRIX* CDXUTSDKMesh::GetWorldMatrix( UINT iFrameIndex )
{
    return &m_pWorldPoseFrameMatrices[iFrameIndex];
}

const D3DXMATRIX* CDXUTSDKMesh::GetInfluenceMatrix( UINT iFrameIndex )
{
    return &m_pTransformedFrameMatrices[iFrameIndex];
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetAnimationKeyFromTime( double fTime )
{
    if( m_pAnimationHeader == NULL )
    {
        return 0;
    }

    UINT iTick = ( UINT )( m_pAnimationHeader->AnimationFPS * fTime );

    iTick = iTick % ( m_pAnimationHeader->NumAnimationKeys - 1 );
    iTick ++;

    return iTick;
}

bool CDXUTSDKMesh::GetAnimationProperties( UINT* pNumKeys, FLOAT* pFrameTime )
{
    if( m_pAnimationHeader == NULL )
    {
        return false;
    }

    *pNumKeys = m_pAnimationHeader->NumAnimationKeys;
    *pFrameTime = 1.0f / (FLOAT)m_pAnimationHeader->AnimationFPS;

    return true;
}

void CDXUTSDKMesh::ComputeTangents()
{

}

void CDXUTSDKMesh::RenderFX(  ID3D11Device* pd3dDevice, 
	ID3D11DeviceContext* pd3dDeviceContext,
	ID3DX11EffectTechnique* pTechnique, 	
	ID3DX11EffectShaderResourceVariable* ptxDiffuse,
	ID3DX11EffectShaderResourceVariable* ptxNormal,
	ID3DX11EffectShaderResourceVariable* ptxSpecular,
	ID3DX11EffectVectorVariable* pvDiffuse, 
	ID3DX11EffectVectorVariable* pvSpecular )
{
	RenderFrame( 0, false, pd3dDevice, pd3dDeviceContext, pTechnique, ptxDiffuse, ptxNormal, ptxSpecular, pvDiffuse, pvSpecular );
}


//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::RenderFrame( UINT iFrame,
	bool bAdjacent,
	ID3D11Device* pd3dDevice, 
	ID3D11DeviceContext* pd3dDeviceContext,
	ID3DX11EffectTechnique* pTechnique, 
	ID3DX11EffectShaderResourceVariable* ptxDiffuse,
	ID3DX11EffectShaderResourceVariable* ptxNormal,
	ID3DX11EffectShaderResourceVariable* ptxSpecular,
	ID3DX11EffectVectorVariable* pvDiffuse, 
	ID3DX11EffectVectorVariable* pvSpecular )
{
	if( !m_pStaticMeshData || !m_pFrameArray )
		return;

	if( m_pFrameArray[iFrame].Mesh != INVALID_MESH )
	{
		RenderMesh( m_pFrameArray[iFrame].Mesh,
			bAdjacent, 
			pd3dDevice,
			pd3dDeviceContext,
			pTechnique, 
			ptxDiffuse,
			ptxNormal,
			ptxSpecular,
			pvDiffuse, 
			pvSpecular );
	}

	// Render our children
	if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
		RenderFrame( m_pFrameArray[iFrame].ChildFrame, bAdjacent, pd3dDevice, pd3dDeviceContext, pTechnique, ptxDiffuse, ptxNormal, ptxSpecular, pvDiffuse, pvSpecular );

	// Render our siblings
	if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
		RenderFrame( m_pFrameArray[iFrame].SiblingFrame, bAdjacent, pd3dDevice, pd3dDeviceContext, pTechnique, ptxDiffuse, ptxNormal, ptxSpecular, pvDiffuse, pvSpecular );
}

#define MAX_D3D11_VERTEX_STREAMS D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
void CDXUTSDKMesh::RenderMesh( UINT iMesh,
	bool bAdjacent,
	ID3D11Device* pd3dDevice, 
	ID3D11DeviceContext* pd3dDeviceContext,
	ID3DX11EffectTechnique* pTechnique, 
	ID3DX11EffectShaderResourceVariable* ptxDiffuse,
	ID3DX11EffectShaderResourceVariable* ptxNormal,
	ID3DX11EffectShaderResourceVariable* ptxSpecular,
	ID3DX11EffectVectorVariable* pvDiffuse, 
	ID3DX11EffectVectorVariable* pvSpecular )
{
	if( 0 < GetOutstandingBufferResources() )
		return;

	SDKMESH_MESH* pMesh = &m_pMeshArray[iMesh];

	UINT Strides[MAX_D3D11_VERTEX_STREAMS];
	UINT Offsets[MAX_D3D11_VERTEX_STREAMS];
	ID3D11Buffer* pVB[MAX_D3D11_VERTEX_STREAMS];

	if( pMesh->NumVertexBuffers > MAX_D3D11_VERTEX_STREAMS )
		return;

	for( UINT64 i=0; i<pMesh->NumVertexBuffers; i++ )
	{
		pVB[i] = m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].pVB11;
		Strides[i] = (UINT)m_pVertexBufferArray[ pMesh->VertexBuffers[i] ].StrideBytes;
		Offsets[i] = 0;
	}

	SDKMESH_INDEX_BUFFER_HEADER* pIndexBufferArray;
	if( bAdjacent )
		pIndexBufferArray = m_pAdjacencyIndexBufferArray;
	else
		pIndexBufferArray = m_pIndexBufferArray;

	ID3D11Buffer* pIB = pIndexBufferArray[ pMesh->IndexBuffer ].pIB11;
	DXGI_FORMAT ibFormat = DXGI_FORMAT_R16_UINT;
	switch( pIndexBufferArray[ pMesh->IndexBuffer ].IndexType )
	{
	case IT_16BIT:
		ibFormat = DXGI_FORMAT_R16_UINT;
		break;
	case IT_32BIT:
		ibFormat = DXGI_FORMAT_R32_UINT;
		break;
	};

	pd3dDeviceContext->IASetVertexBuffers( 0, pMesh->NumVertexBuffers, pVB, Strides, Offsets );
	pd3dDeviceContext->IASetIndexBuffer( pIB, ibFormat, 0 );

	D3DX11_TECHNIQUE_DESC techDesc;
	pTechnique->GetDesc( &techDesc );
	SDKMESH_SUBSET* pSubset = NULL;
	SDKMESH_MATERIAL* pMat = NULL;
	D3D11_PRIMITIVE_TOPOLOGY PrimType;


	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		for( UINT subset = 0; subset < pMesh->NumSubsets; subset++ )
		{
			pSubset = &m_pSubsetArray[ pMesh->pSubsets[subset] ];

			PrimType = GetPrimitiveType11( (SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType );
			if( bAdjacent )
			{
				switch( PrimType )
				{
				case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
					PrimType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
					break;
				case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
					PrimType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
					break;
				case D3D10_PRIMITIVE_TOPOLOGY_LINELIST:
					PrimType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
					break;
				case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP:
					PrimType = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
					break;
				}
			}

			pd3dDeviceContext->IASetPrimitiveTopology( PrimType );

			pMat = &m_pMaterialArray[ pSubset->MaterialID ];
			if( ptxDiffuse && !IsErrorResource(pMat->pDiffuseRV11) )
				ptxDiffuse->SetResource( pMat->pDiffuseRV11 );
			if( ptxNormal && !IsErrorResource(pMat->pNormalRV11) )
				ptxNormal->SetResource( pMat->pNormalRV11 );
			if( ptxSpecular && !IsErrorResource(pMat->pSpecularRV11) )
				ptxSpecular->SetResource( pMat->pSpecularRV11 );
			if( pvDiffuse )
				pvDiffuse->SetFloatVector( pMat->Diffuse );
			if( pvSpecular )
				pvSpecular->SetFloatVector( pMat->Specular );

			pTechnique->GetPassByIndex( p )->Apply(0, pd3dDeviceContext);

			UINT IndexCount = (UINT)pSubset->IndexCount;
			UINT IndexStart = (UINT)pSubset->IndexStart;
			UINT VertexStart = (UINT)pSubset->VertexStart;
			if( bAdjacent )
			{
				IndexCount *= 2;
				IndexStart *= 2;
			}
			pd3dDeviceContext->DrawIndexed( IndexCount, IndexStart, VertexStart );			
		}
	}
}