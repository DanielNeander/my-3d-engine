#include "stdafx.h"
#include <time.h>
#include "GameApp/GameApp.h"
#include "Body.h"
#include "../Mesh.h"
#include "../Mesh_D3D11.h"

// NOTE:  x is right, y is forward, z is up
//
// 3D Camera
//
struct Camera : public CoordFrame
{
	float D, F;	//near and far plane
	noVec3 V;	//velocity
	noVec3 W;	//angular velocity


	Camera()
		: D(0.1f), F(1000)
	{}

	Camera( float d, float f, float a )
		: D(d), F(f)
	{}
};

//
// 3 vertex indices
//
struct Triangle
{
	unsigned short V[3];//vertex indices
	//NOTE:  No more than 64K vertices

	//default
	Triangle()
	{
		//more efficient to not initialize
	}

	//initialize
	Triangle( unsigned short v0, unsigned short v1, unsigned short v2 )
	{
		V[0] = v0;
		V[1] = v1;
		V[2] = v2;
	}
};

//
// Triangles, Vertices, Normals, Topology
//
//NOTE:  No more than 64K triangles or vertices
class BoatGeom : public Mesh_D3D11
{
public:
	Triangle *F;//faces
	unsigned short FCount;//number of faces, face normals

	noVec3 *V;//vertices
	noVec3 *N;//vertex normals
	float *dA;
	unsigned short VCount;

	BoatGeom() 
		:	F(NULL), FCount(0),
		N(NULL), dA(NULL), VCount(0)
	{
		V = NULL;
	}

	~BoatGeom()
	{
		if( F )		delete F;
		if( V )		delete V;
		if( N )		delete N;
		if( dA )	delete dA;
	}
};

//
// 2D Array of floats
//
struct FloatArray2D
{
	float* pData;//pointer to externally allocated data
	const long Width;//array width

	FloatArray2D( float* pd, const long w )
		: pData(pd), Width(w)
	{}

	//indexing operator
	//read-only
	const float* operator [] ( const long i ) const		{ return pData + i*Width; }

	//write-to
	float* operator [] ( const long i )					{ return pData + i*Width; }
};


Camera gCamera;
RigidBody	gSpinningBox( 1, noVec3(1,2,3) );
RigidBody gBoat;
BoatGeom* gBoatGeom;

//water stuff
const long N = 128 + 1;//number of gridpoints along a side
const float c = 2;//wave space, meters/sec
float h; 
const float L = h*(N-1);//width of entire grid
float z[N][N];//z[n] values
float z1[N][N];//z[n-1] and z[n+1] values
float d[N][N];//damping coefficients
FloatArray2D Z( (float*)z, N );//z[n] values
FloatArray2D Z1( (float*)z1, N );//z[n-1] and z[n+1] values
noVec3 V[N][N];//triangle vertices
noVec3 VN[N][N];//vertex normals
int Tri[N-1][N][2];//triangle strip elements

Matrix proj;


/////////////////////////////////////////////////////////////////////////////////////
void initializeBoat()
{
	//Physical Properties
	gBoat = RigidBody(	15000, noVec3(6,4,2) );
	gBoat.position( -5, -5, 0 );

	//Geometry
	gBoatGeom = new BoatGeom;
	//allocate arrays
	gBoatGeom->FCount = 12;
	gBoatGeom->F = new Triangle[gBoatGeom->FCount];//faces

	gBoatGeom->VCount = 8;
	gBoatGeom->V = new noVec3[gBoatGeom->VCount];//vertices
	gBoatGeom->N = new noVec3[gBoatGeom->VCount];//vertex normals
	gBoatGeom->dA = new float[gBoatGeom->VCount];//area patches

	//triangles
	gBoatGeom->F[0] = Triangle(0,1,2);
	gBoatGeom->F[1] = Triangle(0,2,3);
	gBoatGeom->F[2] = Triangle(4,5,6);
	gBoatGeom->F[3] = Triangle(4,6,7);
	gBoatGeom->F[4] = Triangle(4,7,1);
	gBoatGeom->F[5] = Triangle(4,1,0);
	gBoatGeom->F[6] = Triangle(7,6,2);
	gBoatGeom->F[7] = Triangle(7,2,1);
	gBoatGeom->F[8] = Triangle(6,5,3);
	gBoatGeom->F[9] = Triangle(6,3,2);
	gBoatGeom->F[10] = Triangle(5,4,0);
	gBoatGeom->F[11] = Triangle(5,0,3);

	//vertices
	gBoatGeom->V[0] = noVec3( 3, 2, 1);
	gBoatGeom->V[1] = noVec3(-3, 2, 1);
	gBoatGeom->V[2] = noVec3(-3,-2, 1);
	gBoatGeom->V[3] = noVec3( 3,-2, 1);
	gBoatGeom->V[4] = noVec3( 3, 2,-1);
	gBoatGeom->V[5] = noVec3( 3,-2,-1);
	gBoatGeom->V[6] = noVec3(-3,-2,-1);
	gBoatGeom->V[7] = noVec3(-3, 2,-1);

	//vertex normals
	//(pointing outward through
	//the vertices)
	gBoatGeom->N[0] = gBoatGeom->V[0];
	gBoatGeom->N[1] = gBoatGeom->V[1];
	gBoatGeom->N[2] = gBoatGeom->V[2];
	gBoatGeom->N[3] = gBoatGeom->V[3];
	gBoatGeom->N[4] = gBoatGeom->V[4];
	gBoatGeom->N[5] = gBoatGeom->V[5];
	gBoatGeom->N[6] = gBoatGeom->V[6];
	gBoatGeom->N[7] = gBoatGeom->V[7];

	for (int i=0; i < 8; ++i)
		gBoatGeom->N[i].Normalize();

	//area patches (1/8th of
	//total surface area)
	gBoatGeom->dA[0] =
		gBoatGeom->dA[1] =
		gBoatGeom->dA[2] =
		gBoatGeom->dA[3] =
		gBoatGeom->dA[4] =
		gBoatGeom->dA[5] =
		gBoatGeom->dA[6] =
		gBoatGeom->dA[7] = 2*(6*4 + 6*2 + 4*2) / 8;
}

/////////////////////////////////////////////////////////////////////////////////////////
void initializeWater()
{
	long i,j;

	//triangle vertices, normals, z-values
	for( i=0 ; i<N ; i++ )
	{
		for( j=0 ; j<N ; j++ )
		{
			noVec3& v = V[i][j];
			noVec3& n = VN[i][j];

			//vertices, center of
			//grid is at x=0, y=0
			v.x = i*h - L/2;
			v.y = j*h - L/2;
			v.z = 0;

			//normal
			n.z = 2*h;
			//later we will let the openGL
			//driver normalize them for us

			//z-values
			z[i][j] = 0;
			z1[i][j] = 0;

			//damping coefficients
			d[i][j] = 0.9999f;

			//create a circular island in
			//the center of the pool
			const float xc = v.x - 1;
			const float yc = v.y - 1;
			if( (xc*xc + yc*yc) <= 10 )
			{
				d[i][j] = 0;
			}
		}
	}

	//normals at the edge of the
	//grid point straight up
	for( i=0 ; i<N ; i++ )
	{
		VN[i][0].z = 1;
		VN[i][N-1].z = 1;
	}

	for( j=0 ; j<N ; j++ )
	{
		VN[0][j].z = 1;
		VN[N-1][j].z = 1;
	}

	//triangle strip elements
	for( i=0 ; i<N-1; i++ )
	{
		for( j=0 ; j<N; j++ )
		{
			Tri[i][j][0] = i*N + j;
			Tri[i][j][1] = (i+1)*N + j;
		}
	}

	//create a big splash
	z[N/3][N/3] = z1[N/3][N/3] = 10;
}

/////////////////////////////////////////////////////////////////////////////////////////
void animateWater
	(
	FloatArray2D& z,
	FloatArray2D& z1,
	const float dt
	)
{
	//Integrate the solution of the 2D wave equation.

	//OPTIMIZATION:  This code has been written for
	//clarity, not speed!  It would be better to use
	//pointer arithmetic instead of indexing.

	//precalculate coefficients
	const float A = (c*dt/h)*(c*dt/h);
	const float B = 2 - 4*A;
	long i, j;

	//edges are unchanged
	for( i=1 ; i<N-1 ; i++ )
	{
		for( j=1 ; j<N-1 ; j++ )
		{
			//integrate, replacing z[n-1] with z[n+1] in place
			z1[i][j] =	A*( z[i-1][j] + z[i+1][j] + z[i][j-1] + z[i][j+1] )
				+ B*z[i][j] - z1[i][j];

			//apply damping coefficients
			z1[i][j] *= d[i][j];

		}
	}

	//swap pointers
	UnSwap( z.pData, z1.pData );
}

/////////////////////////////////////////////////////////////////////////////////////////
void interpolateWater( const float x, const float y, float& zw, noVec3& nw )
{
	const float rh = 1 / h;
	//fractional index
	float u = (x + 0.5*L) * rh;
	float v = (y + 0.5*L) * rh;
	//lower-left vertex of the enclosing grid cell
	const long i = long( u );
	const long j = long( v );
	//interpolation coefficients
	const float a = u - i;
	const float b = v - j;
	const float ab = a*b;

	//if the position is outside of the grid, give a fake value
	if( i<0 || N<=i || j<0 || N<=j )
	{
		zw = 0;
		nw.x = nw.y = 0;
		nw.z = 1;
	}
	else
	{
		//bilinearly interpolate zw and nw
		zw = (1-a-b+ab) * z[i][j] + (b-ab) * z[i][j+1] + (a-ab) * z[i+1][j] + ab * z[i+1][j+1];
		nw = (1-a-b+ab) * VN[i][j] + (b-ab) * VN[i][j+1] + (a-ab) * VN[i+1][j] + ab * VN[i+1][j+1];
		nw.Normalize();//if the interpolation was spherical, this wouldn't be necessary

		/*
		//DEBUG:  interpolated height should be between z_min and z_max
		float z_min = Min( Min(z[i][j],z[i][j+1]), Min(z[i+1][j],z[i+1][j+1]) );
		float z_max = Max( Max(z[i][j],z[i][j+1]), Max(z[i+1][j],z[i+1][j+1]) );

		if( !(z_min<=zw && zw<=z_max) )
		{
			int kd=0;
		}
		//*/
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void animateBoat( const float dt )
{
	const float k = 1000 * 9.8;//water density * gravity
	noVec3 F_net(0,0,0);//net force
	noVec3 T_net(0,0,0);//net torque

	//gravity
	F_net.z -= 9.8 * gBoat.mass_;

	//buoyancy
	for( long i=0 ; i<gBoatGeom->m_iNumVertices * 3; i+=3 )
	{
		//vector from CM to hull vertex, in world space
		noVec3 v(gBoatGeom->GetVertices()[i], gBoatGeom->GetVertices()[i+1], gBoatGeom->GetVertices()[i+2]);
		const noVec3 r = gBoat.transformVectorToParent( v );
		const noVec3 p = gBoat.position() + r;//hull vertex
		noVec3 nw;//water normal
		float zw;//water height

		//bilinearly interpolate the height
		//and normal of the water surface
		//at the (x,y) position
		interpolateWater( p.x, p.y, zw, nw );

		//only calculate buoyancy
		//for submerged portins
		if( zw > p.z )
		{
			//velocity of hull vertex with respect to water, assuming water is still
			const noVec3 v = gBoat.linearVelocity_ + gBoat.angularVelocity_.Cross(r);
			//drag force is proportional to relative velocity
			const noVec3 Fd = - 2500 * v;//HACK:  magic number
			//buoyant force is proportional to weight of water displaced
			const noVec3 Fb = - k * (zw - p.z) * gBoatGeom->dA[i] * gBoatGeom->N[i].z * nw;
			//total force exerted on this hull vertex
			const noVec3 F = Fd + Fb;

			F_net += F;//add to force
			T_net += r.Cross(F);//add to torque
		}
	}

	const noVec3 dv = (1 / gBoat.mass_) * F_net * dt;
	const noVec3 dw = gBoat.dwdt( T_net ) * dt;

	gBoat.translate( gBoat.linearVelocity_ * dt );
	gBoat.rotate( gBoat.angularVelocity_ * dt );
	gBoat.linearVelocity_ += dv;
	gBoat.angularVelocity_ += dw;
	gBoat.angularVelocity_ *= 0.999f;//HACK: fake energy loss prevents instability
}


void App::LoadPhysics()
{
	h = 2*float(0.1*c* noMath::Sqrt(2));//grid cell width (min for stability, assuming dt<=0.1)

	//init camera (this looked good)
	gCamera.position( 15.8f, 15.2f, 14.5f );	
	gCamera.R[0] = noVec3( 0.75f, 0.66f, 0 );
	gCamera.R[1] = noVec3( -0.58f, 0.66f, -0.48f);
	gCamera.R[2] = noVec3( -0.32f, 0.36f, 0.88f );

	//init spinning box
	
	CreateBox();
	
	gSpinningBox.position( 0, 0, 2.5f );
	gSpinningBox.angularVelocity_ = noVec3( 0.5f, 0.5f, 10.0f );

	const float D = gCamera.D;//near plane
	const float F = gCamera.F;//far plane
	const float Q = F / (F-D);



	proj = MatrixPerspectiveFovLH(noMath::PI * 0.4f, GetAspectRatio(), D, F);
}

void App::RenderPhysics()
{

	static float t0 = (float)clock() / CLOCKS_PER_SEC;
	float t1 = (float)clock() / CLOCKS_PER_SEC;

	float dt = t1 - t0;
	printf("dt=%f\n", dt);

	t0 = t1;//for the next frame

	//HACK:  prevent large time steps
	if( dt>0.1f )
		dt = 0.1f;

	//animate the camera
	gCamera.translate( gCamera.V * dt );

	//rotate the box
	const noVec3 dw = gSpinningBox.dwdt( noVec3(0,0,0) ) * dt;
	gSpinningBox.rotate( gSpinningBox.angularVelocity_ * dt );
	gSpinningBox.angularVelocity_ += dw;
	gSpinningBox.angularVelocity_ *= 0.999f;//HACK: fake energy loss

	{	
		const noVec3 R0 = gCamera.R[0];
		const noVec3 R1 = gCamera.R[2];//swap and negate to match
		const noVec3 R2 = -gCamera.R[1];//OGL's camera convention
		const noVec3& O = gCamera.orig_;//position
		float M[4][4] =
		{
			{ R0.x, R1.x, R2.x, 0 },//X
			{ R0.y, R1.y, R2.y, 0 },//Y
			{ R0.z, R1.z, R2.z, 0 },//Z
			{ -R0*(O), -R1*(O), -R2*(O), 1 }//translation
		};

		memcpy(&g_View, M, SIZE16);
	}


	//HACK:  Add in scale factor
	const noVec3& R0 =  gSpinningBox.R[0];
	const noVec3& R1 =  gSpinningBox.R[1];
	const noVec3& R2 =  gSpinningBox.R[2];
	const noVec3& O = gSpinningBox.orig_;
	float M[4][4] =
	{
		{ R0.x, R0.y, R0.z, 0 },
		{ R1.x, R1.y, R1.z, 0 },
		{ R2.x, R2.y, R2.z, 0 },
		{ O.x, O.y, O.z, 1 },
	};

	memcpy(&g_World, M, SIZE16);

	mat4 proj_;
	memcpy(&proj_, &proj, SIZE16);

	renderer->reset();	
	renderer->setVertexFormat(vf);
	renderer->setVertexBuffer(0, vb);
	renderer->setIndexBuffer(ib);
	renderer->setRasterizerState(cullNone);
	renderer->setShader(shader);
	renderer->setDepthState(noDepthWrite);
	renderer->setBlendState(blendSrcAlpha);
	renderer->setSamplerState("samLinear", linearClamp);
	renderer->setTexture("txDiffuse", rt);
	renderer->setShaderConstant4x4f("World", g_World);
	renderer->setShaderConstant4x4f("View", g_View);
	renderer->setShaderConstant4x4f("Projection", proj_);	

	renderer->apply();

	// DrawIndexed	
	renderer->drawElements(PRIM_TRIANGLES, 0, 36, 0, 24);
}
