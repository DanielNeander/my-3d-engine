#include "stdafx.h"
#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/MD5/MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
//#include "GameApp/Animation/MD5IK.h"
#include <GameApp/MD5/MD5Game.h>
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_Actor.h"
#include "GameApp/Physics/Physics_Parametric.h"
#include <GameApp/Physics/Physics_AF.h>
#include <GameApp/MD5/DeclAF.h>
#include <GameApp/MD5/MD5Af.h>
#include "GameApp/MD5/MD5Entity.h"

void ANIM_CreateAnimFrame( const idRenderModel *model, const idMD5Anim *anim, int numJoints, idJointMat *joints, int time, const noVec3 &offset, bool remove_origin_offset ) {
	int					i;
	frameBlend_t		frame;
	const idMD5Joint	*md5joints;
	int					*index;

	if ( !model || /*model->IsDefaultModel() ||*/ !anim ) {
		return;
	}

	if ( numJoints != model->NumJoints() ) {
		////gameLocal.Error( "ANIM_CreateAnimFrame: different # of joints in renderEntity_t than in model (%s)", model->Name() );
	}

	if ( !model->NumJoints() ) {
		// FIXME: Print out a warning?
		return;
	}

	if ( !joints ) {
		////gameLocal.Error( "ANIM_CreateAnimFrame: NULL joint frame pointer on model (%s)", model->Name() );
	}

	if ( numJoints != anim->NumJoints() ) {
		////gameLocal.Warning( "Model '%s' has different # of joints than anim '%s'", model->Name(), anim->Name() );
		for( i = 0; i < numJoints; i++ ) {
			joints[i].SetRotation( mat3_identity );
			joints[i].SetTranslation( offset );
		}
		return;
	}

	// create index for all joints
	index = ( int * )_alloca16( numJoints * sizeof( int ) );
	for ( i = 0; i < numJoints; i++ ) {
		index[i] = i;
	}

	// create the frame
	anim->ConvertTimeToFrame( time, 1, frame );
	idJointQuat *jointFrame = ( idJointQuat * )_alloca16( numJoints * sizeof( *jointFrame ) );
	anim->GetInterpolatedFrame( frame, jointFrame, index, numJoints );

	// convert joint quaternions to joint matrices
	SIMDProcessor->ConvertJointQuatsToJointMats( joints, jointFrame, numJoints );

	// first joint is always root of entire hierarchy
	if ( remove_origin_offset ) {
		joints[0].SetTranslation( offset );
	} else {
		joints[0].SetTranslation( joints[0].ToVec3() + offset );
	}

	// transform the children
	md5joints = model->GetJoints();
	for( i = 1; i < numJoints; i++ ) {
		joints[i] *= joints[ md5joints[i].parent - md5joints ];
	}

}


idMD5Anim::idMD5Anim()
{
	ref_count	= 0;
	numFrames	= 0;
	numJoints	= 0;
	frameRate	= 24;
	animLength	= 0;
	totaldelta.Zero();
}

idMD5Anim::~idMD5Anim()
{
	Free();

}

void idMD5Anim::Free( void )
{
	numFrames	= 0;
	numJoints	= 0;
	frameRate	= 24;
	animLength	= 0;
	name		= "";

	totaldelta.Zero();

	jointInfo.Clear();
	bounds.Clear();
	componentFrames.Clear();
}

bool idMD5Anim::Reload( void )
{
	idStr filename;

	filename = name;
	Free();

	return LoadAnim( filename );
}

size_t idMD5Anim::Allocated( void ) const
{
	size_t	size = bounds.Allocated() + jointInfo.Allocated() + componentFrames.Allocated() + name.Allocated();
	return size;
}

bool idMD5Anim::LoadAnim( const char *filename )
{
	int		version;
	idLexer	parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS | LEXFL_NOSTRINGCONCAT );
	idToken	token;
	int		i, j;
	int		num;

	if ( !parser.LoadFile( filename ) ) {
		return false;
	}

	Free();

	name = filename;

	parser.ExpectTokenString( MD5_VERSION_STRING );
	version = parser.ParseInt();
	if ( version != MD5_VERSION ) {
		parser.Error( "Invalid version %d.  Should be version %d\n", version, MD5_VERSION );
	}

	// skip the commandline
	parser.ExpectTokenString( "commandline" );
	parser.ReadToken( &token );

	// parse num frames
	parser.ExpectTokenString( "numFrames" );
	numFrames = parser.ParseInt();
	if ( numFrames <= 0 ) {
		parser.Error( "Invalid number of frames: %d", numFrames );
	}

	// parse num joints
	parser.ExpectTokenString( "numJoints" );
	numJoints = parser.ParseInt();
	if ( numJoints <= 0 ) {
		parser.Error( "Invalid number of joints: %d", numJoints );
	}

	// parse frame rate
	parser.ExpectTokenString( "frameRate" );
	frameRate = parser.ParseInt();
	if ( frameRate < 0 ) {
		parser.Error( "Invalid frame rate: %d", frameRate );
	}

	// parse number of animated components
	parser.ExpectTokenString( "numAnimatedComponents" );
	numAnimatedComponents = parser.ParseInt();
	if ( ( numAnimatedComponents < 0 ) || ( numAnimatedComponents > numJoints * 6 ) ) {
		parser.Error( "Invalid number of animated components: %d", numAnimatedComponents );
	}

	// parse the hierarchy
	jointInfo.SetGranularity( 1 );
	jointInfo.SetNum( numJoints );
	parser.ExpectTokenString( "hierarchy" );
	parser.ExpectTokenString( "{" );
	for( i = 0; i < numJoints; i++ ) {
		parser.ReadToken( &token );
		jointInfo[ i ].nameIndex = animationLib.JointIndex( token );

		// parse parent num
		jointInfo[ i ].parentNum = parser.ParseInt();
		if ( jointInfo[ i ].parentNum >= i ) {
			parser.Error( "Invalid parent num: %d", jointInfo[ i ].parentNum );
		}

		if ( ( i != 0 ) && ( jointInfo[ i ].parentNum < 0 ) ) {
			parser.Error( "Animations may have only one root joint" );
		}

		// parse anim bits
		jointInfo[ i ].animBits = parser.ParseInt();
		if ( jointInfo[ i ].animBits & ~63 ) {
			parser.Error( "Invalid anim bits: %d", jointInfo[ i ].animBits );
		}

		// parse first component
		jointInfo[ i ].firstComponent = parser.ParseInt();
		if ( ( numAnimatedComponents > 0 ) && ( ( jointInfo[ i ].firstComponent < 0 ) || ( jointInfo[ i ].firstComponent >= numAnimatedComponents ) ) ) {
			parser.Error( "Invalid first component: %d", jointInfo[ i ].firstComponent );
		}
	}

	parser.ExpectTokenString( "}" );

	// parse bounds
	parser.ExpectTokenString( "bounds" );
	parser.ExpectTokenString( "{" );
	bounds.SetGranularity( 1 );
	bounds.SetNum( numFrames );
	for( i = 0; i < numFrames; i++ ) {
		parser.Parse1DMatrix( 3, bounds[ i ][ 0 ].ToFloatPtr() );
		parser.Parse1DMatrix( 3, bounds[ i ][ 1 ].ToFloatPtr() );
	}
	parser.ExpectTokenString( "}" );

	// parse base frame
	baseFrame.SetGranularity( 1 );
	baseFrame.SetNum( numJoints );
	parser.ExpectTokenString( "baseframe" );
	parser.ExpectTokenString( "{" );
	for( i = 0; i < numJoints; i++ ) {
		idCQuat q;
		parser.Parse1DMatrix( 3, baseFrame[ i ].t.ToFloatPtr() );
		parser.Parse1DMatrix( 3, q.ToFloatPtr() );//baseFrame[ i ].q.ToFloatPtr() );
		baseFrame[ i ].q = q.ToQuat();//.w = baseFrame[ i ].q.CalcW();
	}
	parser.ExpectTokenString( "}" );

	// parse frames
	componentFrames.SetGranularity( 1 );
	componentFrames.SetNum( numAnimatedComponents * numFrames );

	float *componentPtr = componentFrames.Ptr();
	for( i = 0; i < numFrames; i++ ) {
		parser.ExpectTokenString( "frame" );
		num = parser.ParseInt();
		if ( num != i ) {
			parser.Error( "Expected frame number %d", i );
		}
		parser.ExpectTokenString( "{" );

		for( j = 0; j < numAnimatedComponents; j++, componentPtr++ ) {
			*componentPtr = parser.ParseFloat();
		}

		parser.ExpectTokenString( "}" );
	}

	// get total move delta
	if ( !numAnimatedComponents ) {
		totaldelta.Zero();
	} else {
		componentPtr = &componentFrames[ jointInfo[ 0 ].firstComponent ];
		if ( jointInfo[ 0 ].animBits & ANIM_TX ) {
			for( i = 0; i < numFrames; i++ ) {
				componentPtr[ numAnimatedComponents * i ] -= baseFrame[ 0 ].t.x;
			}
			totaldelta.x = componentPtr[ numAnimatedComponents * ( numFrames - 1 ) ];
			componentPtr++;
		} else {
			totaldelta.x = 0.0f;
		}
		if ( jointInfo[ 0 ].animBits & ANIM_TY ) {
			for( i = 0; i < numFrames; i++ ) {
				componentPtr[ numAnimatedComponents * i ] -= baseFrame[ 0 ].t.y;
			}
			totaldelta.y = componentPtr[ numAnimatedComponents * ( numFrames - 1 ) ];
			componentPtr++;
		} else {
			totaldelta.y = 0.0f;
		}
		if ( jointInfo[ 0 ].animBits & ANIM_TZ ) {
			for( i = 0; i < numFrames; i++ ) {
				componentPtr[ numAnimatedComponents * i ] -= baseFrame[ 0 ].t.z;
			}
			totaldelta.z = componentPtr[ numAnimatedComponents * ( numFrames - 1 ) ];
		} else {
			totaldelta.z = 0.0f;
		}
	}
	baseFrame[ 0 ].t.Zero();

	// we don't count last frame because it would cause a 1 frame pause at the end
	animLength = ( ( numFrames - 1 ) * 1000 + frameRate - 1 ) / frameRate;

	// done
	return true;
}

void idMD5Anim::IncreaseRefs( void ) const
{
	ref_count++;
}


void idMD5Anim::DecreaseRefs( void ) const
{
	ref_count--;

}

int idMD5Anim::NumRefs( void ) const
{
	return ref_count;

}

void idMD5Anim::CheckModelHierarchy( const idRenderModel *model ) const
{
	int	i;
	int	jointNum;
	int	parent;

	if ( jointInfo.Num() != model->NumJoints() ) {
		//// gameLocal.Error( "Model '%s' has different # of joints than anim '%s'", model->Name(), name.c_str() );
	}

	const idMD5Joint *modelJoints = model->GetJoints();
	for( i = 0; i < jointInfo.Num(); i++ ) {
		jointNum = jointInfo[ i ].nameIndex;
		if ( modelJoints[ i ].name != animationLib.JointName( jointNum ) ) {
			//// gameLocal.Error( "Model '%s''s joint names don't match anim '%s''s", model->Name(), name.c_str() );
		}
		if ( modelJoints[ i ].parent ) {
			parent = modelJoints[ i ].parent - modelJoints;
		} else {
			parent = -1;
		}
		if ( parent != jointInfo[ i ].parentNum ) {
			//// gameLocal.Error( "Model '%s' has different joint hierarchy than anim '%s'", model->Name(), name.c_str() );
		}
	}
}

void idMD5Anim::GetInterpolatedFrame( frameBlend_t &frame, idJointQuat *joints, const int *index, int numIndexes ) const
{
	int						i, numLerpJoints;
	const float				*frame1;
	const float				*frame2;
	const float				*jointframe1;
	const float				*jointframe2;
	const jointAnimInfo_t	*infoPtr;
	int						animBits;
	idJointQuat				*blendJoints;
	idJointQuat				*jointPtr;
	idJointQuat				*blendPtr;
	int						*lerpIndex;

	// copy the baseframe
	SIMDProcessor->Memcpy( joints, baseFrame.Ptr(), baseFrame.Num() * sizeof( baseFrame[ 0 ] ) );

	if ( !numAnimatedComponents ) {
		// just use the base frame
		return;
	}

	blendJoints = (idJointQuat *)_alloca16( baseFrame.Num() * sizeof( blendPtr[ 0 ] ) );
	lerpIndex = (int *)_alloca16( baseFrame.Num() * sizeof( lerpIndex[ 0 ] ) );
	numLerpJoints = 0;

	frame1 = &componentFrames[ frame.frame1 * numAnimatedComponents ];
	frame2 = &componentFrames[ frame.frame2 * numAnimatedComponents ];

	for ( i = 0; i < numIndexes; i++ ) {
		int j = index[i];
		jointPtr = &joints[j];
		blendPtr = &blendJoints[j];
		infoPtr = &jointInfo[j];

		animBits = infoPtr->animBits;
		if ( animBits ) {

			lerpIndex[numLerpJoints++] = j;

			jointframe1 = frame1 + infoPtr->firstComponent;
			jointframe2 = frame2 + infoPtr->firstComponent;

			switch( animBits & (ANIM_TX|ANIM_TY|ANIM_TZ) ) {
			case 0:
				blendPtr->t = jointPtr->t;
				break;
			case ANIM_TX:
				jointPtr->t.x = jointframe1[0];
				blendPtr->t.x = jointframe2[0];
				blendPtr->t.y = jointPtr->t.y;
				blendPtr->t.z = jointPtr->t.z;
				jointframe1++;
				jointframe2++;
				break;
			case ANIM_TY:
				jointPtr->t.y = jointframe1[0];
				blendPtr->t.y = jointframe2[0];
				blendPtr->t.x = jointPtr->t.x;
				blendPtr->t.z = jointPtr->t.z;
				jointframe1++;
				jointframe2++;
				break;
			case ANIM_TZ:
				jointPtr->t.z = jointframe1[0];
				blendPtr->t.z = jointframe2[0];
				blendPtr->t.x = jointPtr->t.x;
				blendPtr->t.y = jointPtr->t.y;
				jointframe1++;
				jointframe2++;
				break;
			case ANIM_TX|ANIM_TY:
				jointPtr->t.x = jointframe1[0];
				jointPtr->t.y = jointframe1[1];
				blendPtr->t.x = jointframe2[0];
				blendPtr->t.y = jointframe2[1];
				blendPtr->t.z = jointPtr->t.z;
				jointframe1 += 2;
				jointframe2 += 2;
				break;
			case ANIM_TX|ANIM_TZ:
				jointPtr->t.x = jointframe1[0];
				jointPtr->t.z = jointframe1[1];
				blendPtr->t.x = jointframe2[0];
				blendPtr->t.z = jointframe2[1];
				blendPtr->t.y = jointPtr->t.y;
				jointframe1 += 2;
				jointframe2 += 2;
				break;
			case ANIM_TY|ANIM_TZ:
				jointPtr->t.y = jointframe1[0];
				jointPtr->t.z = jointframe1[1];
				blendPtr->t.y = jointframe2[0];
				blendPtr->t.z = jointframe2[1];
				blendPtr->t.x = jointPtr->t.x;
				jointframe1 += 2;
				jointframe2 += 2;
				break;
			case ANIM_TX|ANIM_TY|ANIM_TZ:
				jointPtr->t.x = jointframe1[0];
				jointPtr->t.y = jointframe1[1];
				jointPtr->t.z = jointframe1[2];
				blendPtr->t.x = jointframe2[0];
				blendPtr->t.y = jointframe2[1];
				blendPtr->t.z = jointframe2[2];
				jointframe1 += 3;
				jointframe2 += 3;
				break;
			}

			switch( animBits & (ANIM_QX|ANIM_QY|ANIM_QZ) ) {
			case 0:
				blendPtr->q = jointPtr->q;
				break;
			case ANIM_QX:
				jointPtr->q.x = jointframe1[0];
				blendPtr->q.x = jointframe2[0];
				blendPtr->q.y = jointPtr->q.y;
				blendPtr->q.z = jointPtr->q.z;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QY:
				jointPtr->q.y = jointframe1[0];
				blendPtr->q.y = jointframe2[0];
				blendPtr->q.x = jointPtr->q.x;
				blendPtr->q.z = jointPtr->q.z;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QZ:
				jointPtr->q.z = jointframe1[0];
				blendPtr->q.z = jointframe2[0];
				blendPtr->q.x = jointPtr->q.x;
				blendPtr->q.y = jointPtr->q.y;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QX|ANIM_QY:
				jointPtr->q.x = jointframe1[0];
				jointPtr->q.y = jointframe1[1];
				blendPtr->q.x = jointframe2[0];
				blendPtr->q.y = jointframe2[1];
				blendPtr->q.z = jointPtr->q.z;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QX|ANIM_QZ:
				jointPtr->q.x = jointframe1[0];
				jointPtr->q.z = jointframe1[1];
				blendPtr->q.x = jointframe2[0];
				blendPtr->q.z = jointframe2[1];
				blendPtr->q.y = jointPtr->q.y;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QY|ANIM_QZ:
				jointPtr->q.y = jointframe1[0];
				jointPtr->q.z = jointframe1[1];
				blendPtr->q.y = jointframe2[0];
				blendPtr->q.z = jointframe2[1];
				blendPtr->q.x = jointPtr->q.x;
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			case ANIM_QX|ANIM_QY|ANIM_QZ:
				jointPtr->q.x = jointframe1[0];
				jointPtr->q.y = jointframe1[1];
				jointPtr->q.z = jointframe1[2];
				blendPtr->q.x = jointframe2[0];
				blendPtr->q.y = jointframe2[1];
				blendPtr->q.z = jointframe2[2];
				jointPtr->q.w = jointPtr->q.CalcW();
				blendPtr->q.w = blendPtr->q.CalcW();
				break;
			}
		}
	}

	SIMDProcessor->BlendJoints( joints, blendJoints, frame.backlerp, lerpIndex, numLerpJoints );

	if ( frame.cycleCount ) {
		joints[ 0 ].t += totaldelta * ( float )frame.cycleCount;
	}
}

void idMD5Anim::GetSingleFrame( int framenum, idJointQuat *joints, const int *index, int numIndexes ) const
{
	int						i;
	const float				*frame;
	const float				*jointframe;
	int						animBits;
	idJointQuat				*jointPtr;
	const jointAnimInfo_t	*infoPtr;

	// copy the baseframe
	SIMDProcessor->Memcpy( joints, baseFrame.Ptr(), baseFrame.Num() * sizeof( baseFrame[ 0 ] ) );

	if ( ( framenum == 0 ) || !numAnimatedComponents ) {
		// just use the base frame
		return;
	}

	frame = &componentFrames[ framenum * numAnimatedComponents ];

	for ( i = 0; i < numIndexes; i++ ) {
		int j = index[i];
		jointPtr = &joints[j];
		infoPtr = &jointInfo[j];

		animBits = infoPtr->animBits;
		if ( animBits ) {

			jointframe = frame + infoPtr->firstComponent;

			if ( animBits & (ANIM_TX|ANIM_TY|ANIM_TZ) ) {

				if ( animBits & ANIM_TX ) {
					jointPtr->t.x = *jointframe++;
				}

				if ( animBits & ANIM_TY ) {
					jointPtr->t.y = *jointframe++;
				}

				if ( animBits & ANIM_TZ ) {
					jointPtr->t.z = *jointframe++;
				}
			}

			if ( animBits & (ANIM_QX|ANIM_QY|ANIM_QZ) ) {

				if ( animBits & ANIM_QX ) {
					jointPtr->q.x = *jointframe++;
				}

				if ( animBits & ANIM_QY ) {
					jointPtr->q.y = *jointframe++;
				}

				if ( animBits & ANIM_QZ ) {
					jointPtr->q.z = *jointframe;
				}

				jointPtr->q.w = jointPtr->q.CalcW();
			}
		}
	}
}

int idMD5Anim::Length( void ) const
{
	return animLength;

}

int idMD5Anim::NumFrames( void ) const
{
	return numFrames;

}

int idMD5Anim::NumJoints( void ) const
{
	return numJoints;

}

const noVec3			& idMD5Anim::TotalMovementDelta( void ) const
{
	return totaldelta;

}

const char				* idMD5Anim::Name( void ) const
{
	return name;

}

void idMD5Anim::GetFrameBlend( int framenum, frameBlend_t &frame ) const
{
	frame.cycleCount	= 0;
	frame.backlerp		= 0.0f;
	frame.frontlerp		= 1.0f;

	// frame 1 is first frame
	framenum--;
	if ( framenum < 0 ) {
		framenum = 0;
	} else if ( framenum >= numFrames ) {
		framenum = numFrames - 1;
	}

	frame.frame1 = framenum;
	frame.frame2 = framenum;
}

void idMD5Anim::ConvertTimeToFrame( int time, int cyclecount, frameBlend_t &frame ) const
{
	int frameTime;
	int frameNum;

	if ( numFrames <= 1 ) {
		frame.frame1		= 0;
		frame.frame2		= 0;
		frame.backlerp		= 0.0f;
		frame.frontlerp		= 1.0f;
		frame.cycleCount	= 0;
		return;
	}

	if ( time <= 0 ) {
		frame.frame1		= 0;
		frame.frame2		= 1;
		frame.backlerp		= 0.0f;
		frame.frontlerp		= 1.0f;
		frame.cycleCount	= 0;
		return;
	}

	frameTime			= time * frameRate;
	frameNum			= frameTime / 1000;
	frame.cycleCount	= frameNum / ( numFrames - 1 );

	if ( ( cyclecount > 0 ) && ( frame.cycleCount >= cyclecount ) ) {
		frame.cycleCount	= cyclecount - 1;
		frame.frame1		= numFrames - 1;
		frame.frame2		= frame.frame1;
		frame.backlerp		= 0.0f;
		frame.frontlerp		= 1.0f;
		return;
	}

	frame.frame1 = frameNum % ( numFrames - 1 );
	frame.frame2 = frame.frame1 + 1;
	if ( frame.frame2 >= numFrames ) {
		frame.frame2 = 0;
	}

	frame.backlerp	= ( frameTime % 1000 ) * 0.001f;
	frame.frontlerp	= 1.0f - frame.backlerp;
}

void idMD5Anim::GetOrigin( noVec3 &offset, int time, int cyclecount ) const
{
	frameBlend_t frame;

	offset = baseFrame[ 0 ].t;
	if ( !( jointInfo[ 0 ].animBits & ( ANIM_TX | ANIM_TY | ANIM_TZ ) ) ) {
		// just use the baseframe		
		return;
	}

	ConvertTimeToFrame( time, cyclecount, frame );

	const float *componentPtr1 = &componentFrames[ numAnimatedComponents * frame.frame1 + jointInfo[ 0 ].firstComponent ];
	const float *componentPtr2 = &componentFrames[ numAnimatedComponents * frame.frame2 + jointInfo[ 0 ].firstComponent ];

	if ( jointInfo[ 0 ].animBits & ANIM_TX ) {
		offset.x = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
		componentPtr1++;
		componentPtr2++;
	}

	if ( jointInfo[ 0 ].animBits & ANIM_TY ) {
		offset.y = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
		componentPtr1++;
		componentPtr2++;
	}

	if ( jointInfo[ 0 ].animBits & ANIM_TZ ) {
		offset.z = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
	}

	if ( frame.cycleCount ) {
		offset += totaldelta * ( float )frame.cycleCount;
	}
}

void idMD5Anim::GetOriginRotation( noQuat &rotation, int time, int cyclecount ) const
{
	frameBlend_t	frame;
	int				animBits;

	animBits = jointInfo[ 0 ].animBits;
	if ( !( animBits & ( ANIM_QX | ANIM_QY | ANIM_QZ ) ) ) {
		// just use the baseframe		
		rotation = baseFrame[ 0 ].q;
		return;
	}

	ConvertTimeToFrame( time, cyclecount, frame );

	const float	*jointframe1 = &componentFrames[ numAnimatedComponents * frame.frame1 + jointInfo[ 0 ].firstComponent ];
	const float	*jointframe2 = &componentFrames[ numAnimatedComponents * frame.frame2 + jointInfo[ 0 ].firstComponent ];

	if ( animBits & ANIM_TX ) {
		jointframe1++;
		jointframe2++;
	}

	if ( animBits & ANIM_TY ) {
		jointframe1++;
		jointframe2++;
	}

	if ( animBits & ANIM_TZ ) {
		jointframe1++;
		jointframe2++;
	}

	noQuat q1;
	noQuat q2;

	switch( animBits & (ANIM_QX|ANIM_QY|ANIM_QZ) ) {
	case ANIM_QX:
		q1.x = jointframe1[0];
		q2.x = jointframe2[0];
		q1.y = baseFrame[ 0 ].q.y;
		q2.y = q1.y;
		q1.z = baseFrame[ 0 ].q.z;
		q2.z = q1.z;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QY:
		q1.y = jointframe1[0];
		q2.y = jointframe2[0];
		q1.x = baseFrame[ 0 ].q.x;
		q2.x = q1.x;
		q1.z = baseFrame[ 0 ].q.z;
		q2.z = q1.z;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QZ:
		q1.z = jointframe1[0];
		q2.z = jointframe2[0];
		q1.x = baseFrame[ 0 ].q.x;
		q2.x = q1.x;
		q1.y = baseFrame[ 0 ].q.y;
		q2.y = q1.y;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QX|ANIM_QY:
		q1.x = jointframe1[0];
		q1.y = jointframe1[1];
		q2.x = jointframe2[0];
		q2.y = jointframe2[1];
		q1.z = baseFrame[ 0 ].q.z;
		q2.z = q1.z;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QX|ANIM_QZ:
		q1.x = jointframe1[0];
		q1.z = jointframe1[1];
		q2.x = jointframe2[0];
		q2.z = jointframe2[1];
		q1.y = baseFrame[ 0 ].q.y;
		q2.y = q1.y;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QY|ANIM_QZ:
		q1.y = jointframe1[0];
		q1.z = jointframe1[1];
		q2.y = jointframe2[0];
		q2.z = jointframe2[1];
		q1.x = baseFrame[ 0 ].q.x;
		q2.x = q1.x;
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	case ANIM_QX|ANIM_QY|ANIM_QZ:
		q1.x = jointframe1[0];
		q1.y = jointframe1[1];
		q1.z = jointframe1[2];
		q2.x = jointframe2[0];
		q2.y = jointframe2[1];
		q2.z = jointframe2[2];
		q1.w = q1.CalcW();
		q2.w = q2.CalcW();
		break;
	}

	rotation.Slerp( q1, q2, frame.backlerp );
}

void idMD5Anim::GetBounds( idBounds &bnds, int time, int cyclecount ) const
{
	frameBlend_t	frame;
	noVec3			offset;

	ConvertTimeToFrame( time, cyclecount, frame );

	bnds = bounds[ frame.frame1 ];
	bnds.AddBounds( bounds[ frame.frame2 ] );

	// origin position
	offset = baseFrame[ 0 ].t;
	if ( jointInfo[ 0 ].animBits & ( ANIM_TX | ANIM_TY | ANIM_TZ ) ) {
		const float *componentPtr1 = &componentFrames[ numAnimatedComponents * frame.frame1 + jointInfo[ 0 ].firstComponent ];
		const float *componentPtr2 = &componentFrames[ numAnimatedComponents * frame.frame2 + jointInfo[ 0 ].firstComponent ];

		if ( jointInfo[ 0 ].animBits & ANIM_TX ) {
			offset.x = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
			componentPtr1++;
			componentPtr2++;
		}

		if ( jointInfo[ 0 ].animBits & ANIM_TY ) {
			offset.y = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
			componentPtr1++;
			componentPtr2++;
		}

		if ( jointInfo[ 0 ].animBits & ANIM_TZ ) {
			offset.z = *componentPtr1 * frame.frontlerp + *componentPtr2 * frame.backlerp;
		}
	}

	bnds[ 0 ] -= offset;
	bnds[ 1 ] -= offset;
}



idAnimator::idAnimator()
{
	int	i, j;

	modelDef				= NULL;
	//entity					= NULL;
	numJoints				= 0;
	joints					= NULL;
	lastTransformTime		= -1;
	stoppedAnimatingUpdate	= false;
	removeOriginOffset		= false;
	forceUpdate				= false;

	frameBounds.Clear();

	AFPoseJoints.SetGranularity( 1 );
	AFPoseJointMods.SetGranularity( 1 );
	AFPoseJointFrame.SetGranularity( 1 );

	ClearAFPose();

	for( i = ANIMCHANNEL_ALL; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++ ) {
			channels[ i ][ j ].Reset( NULL );
		}
	}
}

idAnimator::~idAnimator()
{
	FreeData();

}

size_t idAnimator::Allocated( void ) const
{
	size_t	size;

	size = jointMods.Allocated() + numJoints * sizeof( joints[0] ) + jointMods.Num() * sizeof( jointMods[ 0 ] ) + AFPoseJointMods.Allocated() + AFPoseJointFrame.Allocated() + AFPoseJoints.Allocated();

	return size;
}

size_t idAnimator::Size( void ) const
{
	return sizeof( *this ) + Allocated();

}

void idAnimator::RemoveOriginOffset( bool remove )
{
	removeOriginOffset = remove;

}

bool idAnimator::RemoveOrigin( void ) const
{
	return removeOriginOffset;

}

void idAnimator::GetJointList( const char *jointnames, idList<jointHandle_t> &jointList ) const
{
	if ( modelDef ) {
		modelDef->GetJointList( jointnames, jointList );
	}
}

int idAnimator::NumAnims( void ) const
{
	if ( !modelDef ) {
		return 0;
	}

	return modelDef->NumAnims();
}

const idAnim				* idAnimator::GetAnim( int index ) const
{
	if ( !modelDef ) {
		return NULL;
	}

	return modelDef->GetAnim( index );
}

int idAnimator::GetAnim( const char *name ) const
{
	if ( !modelDef ) {
		return 0;
	}

	return modelDef->GetAnim( name );
}

bool idAnimator::HasAnim( const char *name ) const
{
	if ( !modelDef ) {
		return false;
	}

	return modelDef->HasAnim( name );
}

void idAnimator::ServiceAnims( int fromtime, int totime )
{
	int			i, j;
	idAnimBlend	*blend;

	if ( !modelDef ) {
		return;
	}

	if ( modelDef->ModelHandle() ) {
		blend = channels[ 0 ];
		for( i = 0; i < ANIM_NumAnimChannels; i++ ) {
			for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
				//blend->CallFrameCommands( entity, fromtime, totime );
			}
		}
	}

	if ( !IsAnimating( totime ) ) {
		stoppedAnimatingUpdate = true;
		if ( entity ) {
			entity->BecomeInactive( TH_ANIMATE );

			// present one more time with stopped animations so the renderer can properly recreate interactions
			entity->BecomeActive( TH_UPDATEVISUALS );
		}
	}
}

bool idAnimator::IsAnimating( int currentTime ) const
{
	int					i, j;
	const idAnimBlend	*blend;

	if ( !modelDef || !modelDef->ModelHandle() ) {
		return false;
	}

	// if animating with an articulated figure
	if ( AFPoseJoints.Num() && currentTime <= AFPoseTime ) {
		return true;
	}

	blend = channels[ 0 ];
	for( i = 0; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
			if ( !blend->IsDone( currentTime ) ) {
				return true;
			}
		}
	}

	return false;
}

void idAnimator::GetJoints( int *numJoints, idJointMat **jointsPtr )
{
	*numJoints	= this->numJoints;
	*jointsPtr	= this->joints;
}

int idAnimator::NumJoints( void ) const
{
	return numJoints;

}

/*
=====================
idAnimator::GetFirstChild
=====================
*/
jointHandle_t idAnimator::GetFirstChild( const char *name ) const {
	return GetFirstChild( GetJointHandle( name ) );
}

/*
=====================
idAnimator::GetFirstChild
=====================
*/
jointHandle_t idAnimator::GetFirstChild( jointHandle_t jointnum ) const {
	int					i;
	int					num;
	const jointInfo_t	*joint;

	if ( !modelDef ) {
		return INVALID_JOINT;
	}

	num = modelDef->NumJoints();
	if ( !num ) {
		return jointnum;
	}
	joint = modelDef->GetJoint( 0 );
	for( i = 0; i < num; i++, joint++ ) {
		if ( joint->parentNum == jointnum ) {
			return ( jointHandle_t )joint->num;
		}
	}
	return jointnum;
}

const idDeclModelDef		* idAnimator::ModelDef( void ) const
{
	return modelDef;

}

void idAnimator::ForceUpdate( void )
{
	lastTransformTime = -1;
	forceUpdate = true;
}

/*
=====================
idAnimator::ModelHandle
=====================
*/
idRenderModel *idAnimator::ModelHandle( void ) const {
	if ( !modelDef ) {
		return NULL;
	}

	return modelDef->ModelHandle();
}

void idAnimator::ClearForceUpdate( void )
{
	forceUpdate = false;

}

bool idAnimator::CreateFrame( int currentTime, bool force )
{
	int					i, j;
	int					numJoints;
	int					parentNum;
	bool				hasAnim;
	bool				debugInfo = false;
	float				baseBlend;
	float				blendWeight;
	const idAnimBlend *	blend;
	const int *			jointParent;
	const jointMod_t *	jointMod;
	const idJointQuat *	defaultPose;

	static idCVar		r_showSkel( "r_showSkel", "0", CVAR_RENDERER | CVAR_INTEGER, "", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );

	//if ( // gameLocal.inCinematic && // gameLocal.skipCinematic ) {
		//return false;
	//}

	if ( !modelDef || !modelDef->ModelHandle() ) {
		return false;
	}

	if ( !force && !r_showSkel.GetInteger() ) {
		if ( lastTransformTime == currentTime ) {
			return false;
		}
		if ( lastTransformTime != -1 && !stoppedAnimatingUpdate && !IsAnimating( currentTime ) ) {
			return false;
		}
	}

	lastTransformTime = currentTime;
	stoppedAnimatingUpdate = false;

	//if ( entity && ( ( g_debugAnim.GetInteger() == entity->entityNumber ) || ( g_debugAnim.GetInteger() == -2 ) ) ) {
	//	debugInfo = true;
	//	// gameLocal.Printf( "---------------\n%d: entity '%s':\n", // gameLocal.time, entity->GetName() );
	//	// gameLocal.Printf( "model '%s':\n", modelDef->GetModelName() );
	//} else {
	//	debugInfo = false;
	//}

	// init the joint buffer
	if ( AFPoseJoints.Num() ) {
		// initialize with AF pose anim for the case where there are no other animations and no AF pose joint modifications
		defaultPose = AFPoseJointFrame.Ptr();
	} else {
		defaultPose = modelDef->GetDefaultPose();
	}

	if ( !defaultPose ) {
		//// gameLocal.Warning( "idAnimator::CreateFrame: no defaultPose on '%s'", modelDef->Name() );
		return false;
	}

	numJoints = modelDef->Joints().Num();
	idJointQuat *jointFrame = ( idJointQuat * )_alloca16( numJoints * sizeof( jointFrame[0] ) );
	SIMDProcessor->Memcpy( jointFrame, defaultPose, numJoints * sizeof( jointFrame[0] ) );

	hasAnim = false;

	// blend the all channel
	baseBlend = 0.0f;
	blend = channels[ ANIMCHANNEL_ALL ];
	for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
		if ( blend->BlendAnim( currentTime, ANIMCHANNEL_ALL, numJoints, jointFrame, baseBlend, removeOriginOffset, false, debugInfo ) ) {
			hasAnim = true;
			if ( baseBlend >= 1.0f ) {
				break;
			}
		}
	}

	// only blend other channels if there's enough space to blend into
	if ( baseBlend < 1.0f ) {
		for( i = ANIMCHANNEL_ALL + 1; i < ANIM_NumAnimChannels; i++ ) {
			if ( !modelDef->NumJointsOnChannel( i ) ) {
				continue;
			}
			if ( i == ANIMCHANNEL_EYELIDS ) {
				// eyelids blend over any previous anims, so skip it and blend it later
				continue;
			}
			blendWeight = baseBlend;
			blend = channels[ i ];
			for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
				if ( blend->BlendAnim( currentTime, i, numJoints, jointFrame, blendWeight, removeOriginOffset, false, debugInfo ) ) {
					hasAnim = true;
					if ( blendWeight >= 1.0f ) {
						// fully blended
						break;
					}
				}
			}

			if ( debugInfo && !AFPoseJoints.Num() && !blendWeight ) {
				// gameLocal.Printf( "%d: %s using default pose in model '%s'\n", // gameLocal.time, channelNames[ i ], modelDef->GetModelName() );
			}
		}
	}

	// blend in the eyelids
	if ( modelDef->NumJointsOnChannel( ANIMCHANNEL_EYELIDS ) ) {
		blend = channels[ ANIMCHANNEL_EYELIDS ];
		blendWeight = baseBlend;
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
			if ( blend->BlendAnim( currentTime, ANIMCHANNEL_EYELIDS, numJoints, jointFrame, blendWeight, removeOriginOffset, true, debugInfo ) ) {
				hasAnim = true;
				if ( blendWeight >= 1.0f ) {
					// fully blended
					break;
				}
			}
		}
	}

	// blend the articulated figure pose
	if ( BlendAFPose( jointFrame ) ) {
		hasAnim = true;
	}

	if ( !hasAnim && !jointMods.Num() ) {
		// no animations were updated
		return false;
	}

	// convert the joint quaternions to rotation matrices
	SIMDProcessor->ConvertJointQuatsToJointMats( joints, jointFrame, numJoints );

	// check if we need to modify the origin
	if ( jointMods.Num() && ( jointMods[0]->jointnum == 0 ) ) {
		jointMod = jointMods[0];

		switch( jointMod->transform_axis ) {
		case JOINTMOD_NONE:
			break;

		case JOINTMOD_LOCAL:
			joints[0].SetRotation( jointMod->mat * joints[0].ToMat3() );
			break;

		case JOINTMOD_WORLD:
			joints[0].SetRotation( joints[0].ToMat3() * jointMod->mat );
			break;

		case JOINTMOD_LOCAL_OVERRIDE:
		case JOINTMOD_WORLD_OVERRIDE:
			joints[0].SetRotation( jointMod->mat );
			break;
		}

		switch( jointMod->transform_pos ) {
		case JOINTMOD_NONE:
			break;

		case JOINTMOD_LOCAL:
			joints[0].SetTranslation( joints[0].ToVec3() + jointMod->pos );
			break;

		case JOINTMOD_LOCAL_OVERRIDE:
		case JOINTMOD_WORLD:
		case JOINTMOD_WORLD_OVERRIDE:
			joints[0].SetTranslation( jointMod->pos );
			break;
		}
		j = 1;
	} else {
		j = 0;
	}

	// add in the model offset
	joints[0].SetTranslation( joints[0].ToVec3() + modelDef->GetVisualOffset() );

	// pointer to joint info
	jointParent = modelDef->JointParents();

	// add in any joint modifications
	for( i = 1; j < jointMods.Num(); j++, i++ ) {
		jointMod = jointMods[j];

		// transform any joints preceding the joint modifier
		SIMDProcessor->TransformJoints( joints, jointParent, i, jointMod->jointnum - 1 );
		i = jointMod->jointnum;

		parentNum = jointParent[i];

		// modify the axis
		switch( jointMod->transform_axis ) {
		case JOINTMOD_NONE:
			joints[i].SetRotation( joints[i].ToMat3() * joints[ parentNum ].ToMat3() );
			break;

		case JOINTMOD_LOCAL:
			joints[i].SetRotation( jointMod->mat * ( joints[i].ToMat3() * joints[parentNum].ToMat3() ) );
			break;

		case JOINTMOD_LOCAL_OVERRIDE:
			joints[i].SetRotation( jointMod->mat * joints[parentNum].ToMat3() );
			break;

		case JOINTMOD_WORLD:
			joints[i].SetRotation( ( joints[i].ToMat3() * joints[parentNum].ToMat3() ) * jointMod->mat );
			break;

		case JOINTMOD_WORLD_OVERRIDE:
			joints[i].SetRotation( jointMod->mat );
			break;
		}

		// modify the position
		switch( jointMod->transform_pos ) {
		case JOINTMOD_NONE:
			joints[i].SetTranslation( joints[parentNum].ToVec3() + joints[i].ToVec3() * joints[parentNum].ToMat3() );
			break;

		case JOINTMOD_LOCAL:
			joints[i].SetTranslation( joints[parentNum].ToVec3() + ( joints[i].ToVec3() + jointMod->pos ) * joints[parentNum].ToMat3() );
			break;

		case JOINTMOD_LOCAL_OVERRIDE:
			joints[i].SetTranslation( joints[parentNum].ToVec3() + jointMod->pos * joints[parentNum].ToMat3() );
			break;

		case JOINTMOD_WORLD:
			joints[i].SetTranslation( joints[parentNum].ToVec3() + joints[i].ToVec3() * joints[parentNum].ToMat3() + jointMod->pos );
			break;

		case JOINTMOD_WORLD_OVERRIDE:
			joints[i].SetTranslation( jointMod->pos );
			break;
		}
	}

	// transform the rest of the hierarchy
	SIMDProcessor->TransformJoints( joints, jointParent, i, numJoints - 1 );

	return true;
}

bool idAnimator::FrameHasChanged( int currentTime ) const
{
	int					i, j;
	const idAnimBlend	*blend;

	if ( !modelDef || !modelDef->ModelHandle() ) {
		return false;
	}

	// if animating with an articulated figure
	if ( AFPoseJoints.Num() && currentTime <= AFPoseTime ) {
		return true;
	}

	blend = channels[ 0 ];
	for( i = 0; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
			if ( blend->FrameHasChanged( currentTime ) ) {
				return true;
			}
		}
	}

	if ( forceUpdate && IsAnimating( currentTime ) ) {
		return true;
	}

	return false;
}

void idAnimator::GetDelta( int fromtime, int totime, noVec3 &delta ) const
{
	int					i;
	const idAnimBlend	*blend;
	float				blendWeight;

	if ( !modelDef || !modelDef->ModelHandle() || ( fromtime == totime ) ) {
		delta.Zero();
		return;
	}

	delta.Zero();
	blendWeight = 0.0f;

	blend = channels[ ANIMCHANNEL_ALL ];
	for( i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
		blend->BlendDelta( fromtime, totime, delta, blendWeight );
	}

	if ( modelDef->Joints()[ 0 ].channel ) {
		blend = channels[ modelDef->Joints()[ 0 ].channel ];
		for( i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
			blend->BlendDelta( fromtime, totime, delta, blendWeight );
		}
	}
}

bool idAnimator::GetDeltaRotation( int fromtime, int totime, noMat3 &delta ) const
{
	int					i;
	const idAnimBlend	*blend;
	float				blendWeight;
	noQuat				q;

	if ( !modelDef || !modelDef->ModelHandle() || ( fromtime == totime ) ) {
		delta.Identity();
		return false;
	}

	q.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	blendWeight = 0.0f;

	blend = channels[ ANIMCHANNEL_ALL ];
	for( i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
		blend->BlendDeltaRotation( fromtime, totime, q, blendWeight );
	}

	if ( modelDef->Joints()[ 0 ].channel ) {
		blend = channels[ modelDef->Joints()[ 0 ].channel ];
		for( i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
			blend->BlendDeltaRotation( fromtime, totime, q, blendWeight );
		}
	}

	if ( blendWeight > 0.0f ) {
		delta = q.ToMat3();
		return true;
	} else {
		delta.Identity();
		return false;
	}
}

void idAnimator::GetOrigin( int currentTime, noVec3 &pos ) const
{
	const idAnimBlend	*blend;
	float				blendWeight;

	if ( !modelDef || !modelDef->ModelHandle() ) {
		pos.Zero();
		return;
	}

	pos.Zero();
	blendWeight = 0.0f;

	blend = channels[ ANIMCHANNEL_ALL ];
	for( int i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
		blend->BlendOrigin( currentTime, pos, blendWeight, removeOriginOffset );
	}

	if ( modelDef->Joints()[ 0 ].channel ) {
		blend = channels[ modelDef->Joints()[ 0 ].channel ];
		for( int i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
			blend->BlendOrigin( currentTime, pos, blendWeight, removeOriginOffset );
		}
	}

	pos += modelDef->GetVisualOffset();
}

bool idAnimator::GetBounds( int currentTime, idBounds &bounds )
{
	int					i, j;
	const idAnimBlend	*blend;
	int					count;

	if ( !modelDef || !modelDef->ModelHandle() ) {
		return false;
	}

	if ( AFPoseJoints.Num() ) {
		bounds = AFPoseBounds;
		count = 1;
	} else {
		bounds.Clear();
		count = 0;
	}

	blend = channels[ 0 ];
	for( i = ANIMCHANNEL_ALL; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++, blend++ ) {
			if ( blend->AddBounds( currentTime, bounds, removeOriginOffset ) ) {
				count++;
			}
		}
	}

	if ( !count ) {
		if ( !frameBounds.IsCleared() ) {
			bounds = frameBounds;
			return true;
		} else {
			bounds.Zero();
			return false;
		}
	}

	bounds.TranslateSelf( modelDef->GetVisualOffset() );

	/*if ( g_debugBounds.GetBool() ) {
		if ( bounds[1][0] - bounds[0][0] > 2048 || bounds[1][1] - bounds[0][1] > 2048 ) {
			if ( entity ) {
				// gameLocal.Warning( "big frameBounds on entity '%s' with model '%s': %f,%f", entity->name.c_str(), modelDef->ModelHandle()->Name(), bounds[1][0] - bounds[0][0], bounds[1][1] - bounds[0][1] );
			} else {
				// gameLocal.Warning( "big frameBounds on model '%s': %f,%f", modelDef->ModelHandle()->Name(), bounds[1][0] - bounds[0][0], bounds[1][1] - bounds[0][1] );
			}
		}
	}*/

	frameBounds = bounds;

	return true;
}

idAnimBlend					* idAnimator::CurrentAnim( int channelNum )
{
	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) ) {
		//// gameLocal.Error( "idAnimator::CurrentAnim : channel out of range" );
	}

	return &channels[ channelNum ][ 0 ];
}

void idAnimator::Clear( int channelNum, int currentTime, int cleartime )
{
	int			i;
	idAnimBlend	*blend;

	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) ) {
		//// gameLocal.Error( "idAnimator::Clear : channel out of range" );
	}

	blend = channels[ channelNum ];
	for( i = 0; i < ANIM_MaxAnimsPerChannel; i++, blend++ ) {
		blend->Clear( currentTime, cleartime );
	}
	ForceUpdate();
}

void idAnimator::SetFrame( int channelNum, int animNum, int frame, int currentTime, int blendTime )
{
	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) ) {
	//	// gameLocal.Error( "idAnimator::SetFrame : channel out of range" );
	}

	if ( !modelDef || !modelDef->GetAnim( animNum ) ) {
		return;
	}

	PushAnims( channelNum, currentTime, blendTime );
	channels[ channelNum ][ 0 ].SetFrame( modelDef, animNum, frame, currentTime, blendTime );
	/*if ( entity ) {
		entity->BecomeActive( TH_ANIMATE );
	}*/
}

void idAnimator::CycleAnim( int channelNum, int animNum, int currentTime, int blendTime )
{
	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) ) {
		//// gameLocal.Error( "idAnimator::CycleAnim : channel out of range" );
	}

	if ( !modelDef || !modelDef->GetAnim( animNum ) ) {
		return;
	}

	PushAnims( channelNum, currentTime, blendTime );
	channels[ channelNum ][ 0 ].CycleAnim( modelDef, animNum, currentTime, blendTime );
	if ( entity ) {
		entity->BecomeActive( TH_ANIMATE );
	}
}

void idAnimator::PlayAnim( int channelNum, int animNum, int currentTime, int blendTime )
{
	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) ) {
		//// gameLocal.Error( "idAnimator::PlayAnim : channel out of range" );
	}

	if ( !modelDef || !modelDef->GetAnim( animNum ) ) {
		return;
	}

	PushAnims( channelNum, currentTime, blendTime );
	channels[ channelNum ][ 0 ].PlayAnim( modelDef, animNum, currentTime, blendTime );
	if ( entity ) {
		entity->BecomeActive( TH_ANIMATE );
	}
}

void idAnimator::SyncAnimChannels( int channelNum, int fromChannelNum, int currentTime, int blendTime )
{
	if ( ( channelNum < 0 ) || ( channelNum >= ANIM_NumAnimChannels ) || ( fromChannelNum < 0 ) || ( fromChannelNum >= ANIM_NumAnimChannels ) ) {
		//// gameLocal.Error( "idAnimator::SyncToChannel : channel out of range" );
	}

	idAnimBlend &fromBlend = channels[ fromChannelNum ][ 0 ];
	idAnimBlend &toBlend = channels[ channelNum ][ 0 ];

	float weight = fromBlend.blendEndValue;
	if ( ( fromBlend.Anim() != toBlend.Anim() ) || ( fromBlend.GetStartTime() != toBlend.GetStartTime() ) || ( fromBlend.GetEndTime() != toBlend.GetEndTime() ) ) {
		PushAnims( channelNum, currentTime, blendTime );
		toBlend = fromBlend;
		toBlend.blendStartValue = 0.0f;
		toBlend.blendEndValue = 0.0f;
	}
	toBlend.SetWeight( weight, currentTime - 1, blendTime );

	// disable framecommands on the current channel so that commands aren't called twice
	toBlend.AllowFrameCommands( false );
}

void idAnimator::SetJointPos( jointHandle_t jointnum, jointModTransform_t transform_type, const noVec3 &pos )
{
	int i;
	jointMod_t *jointMod;

	if ( !modelDef || !modelDef->ModelHandle() || ( jointnum < 0 ) || ( jointnum >= numJoints ) ) {
		return;
	}

	jointMod = NULL;
	for( i = 0; i < jointMods.Num(); i++ ) {
		if ( jointMods[ i ]->jointnum == jointnum ) {
			jointMod = jointMods[ i ];
			break;
		} else if ( jointMods[ i ]->jointnum > jointnum ) {
			break;
		}
	}

	if ( !jointMod ) {
		jointMod = new jointMod_t;
		jointMod->jointnum = jointnum;
		jointMod->mat.Identity();
		jointMod->transform_axis = JOINTMOD_NONE;
		jointMods.Insert( jointMod, i );
	}

	jointMod->pos = pos;
	jointMod->transform_pos = transform_type;

	if ( entity ) {
		entity->BecomeActive( TH_ANIMATE );
	}
	ForceUpdate();
}

void idAnimator::SetJointAxis( jointHandle_t jointnum, jointModTransform_t transform_type, const noMat3 &mat )
{
	int i;
	jointMod_t *jointMod;

	if ( !modelDef || !modelDef->ModelHandle() || ( jointnum < 0 ) || ( jointnum >= numJoints ) ) {
		return;
	}

	jointMod = NULL;
	for( i = 0; i < jointMods.Num(); i++ ) {
		if ( jointMods[ i ]->jointnum == jointnum ) {
			jointMod = jointMods[ i ];
			break;
		} else if ( jointMods[ i ]->jointnum > jointnum ) {
			break;
		}
	}

	if ( !jointMod ) {
		jointMod = new jointMod_t;
		jointMod->jointnum = jointnum;
		jointMod->pos.Zero();
		jointMod->transform_pos = JOINTMOD_NONE;
		jointMods.Insert( jointMod, i );
	}

	jointMod->mat = mat;
	jointMod->transform_axis = transform_type;

	if ( entity ) {
		entity->BecomeActive( TH_ANIMATE );
	}
	ForceUpdate();
}

void idAnimator::ClearJoint( jointHandle_t jointnum )
{
	int i;

	if ( !modelDef || !modelDef->ModelHandle() || ( jointnum < 0 ) || ( jointnum >= numJoints ) ) {
		return;
	}

	for( i = 0; i < jointMods.Num(); i++ ) {
		if ( jointMods[ i ]->jointnum == jointnum ) {
			delete jointMods[ i ];
			jointMods.RemoveIndex( i );
			ForceUpdate();
			break;
		} else if ( jointMods[ i ]->jointnum > jointnum ) {
			break;
		}
	}
}

void idAnimator::ClearAllJoints( void )
{
	if ( jointMods.Num() ) {
		ForceUpdate();
	}
	jointMods.DeleteContents( true );
}

void idAnimator::InitAFPose( void )
{
	if ( !modelDef ) {
		return;
	}

	AFPoseJoints.SetNum( modelDef->Joints().Num(), false );
	AFPoseJoints.SetNum( 0, false );
	AFPoseJointMods.SetNum( modelDef->Joints().Num(), false );
	AFPoseJointFrame.SetNum( modelDef->Joints().Num(), false );
}

void idAnimator::SetAFPoseJointMod( const jointHandle_t jointNum, const AFJointModType_t mod, const noMat3 &axis, const noVec3 &origin )
{
	AFPoseJointMods[jointNum].mod = mod;
	AFPoseJointMods[jointNum].axis = axis;
	AFPoseJointMods[jointNum].origin = origin;

	int index = idBinSearch_GreaterEqual<int>( AFPoseJoints.Ptr(), AFPoseJoints.Num(), jointNum );
	if ( index >= AFPoseJoints.Num() || jointNum != AFPoseJoints[index] ) {
		AFPoseJoints.Insert( jointNum, index );
	}
}

void idAnimator::FinishAFPose( int animNum, const idBounds &bounds, const int time )
{
	int					i, j;
	int					numJoints;
	int					parentNum;
	int					jointMod;
	int					jointNum;
	const int *			jointParent;

	if ( !modelDef ) {
		return;
	}

	const idAnim *anim = modelDef->GetAnim( animNum );
	if ( !anim ) {
		return;
	}

	numJoints = modelDef->Joints().Num();
	if ( !numJoints ) {
		return;
	}

	idRenderModel		*md5 = modelDef->ModelHandle();
	const idMD5Anim		*md5anim = anim->MD5Anim( 0 );

	if ( numJoints != md5anim->NumJoints() ) {
		//// gameLocal.Warning( "Model '%s' has different # of joints than anim '%s'", md5->Name(), md5anim->Name() );
		return;
	}

	idJointQuat *jointFrame = ( idJointQuat * )_alloca16( numJoints * sizeof( *jointFrame ) );
	md5anim->GetSingleFrame( 0, jointFrame, modelDef->GetChannelJoints( ANIMCHANNEL_ALL ), modelDef->NumJointsOnChannel( ANIMCHANNEL_ALL ) );

	if ( removeOriginOffset ) {
#ifdef VELOCITY_MOVE
		jointFrame[ 0 ].t.x = 0.0f;
#else
		jointFrame[ 0 ].t.Zero();
#endif
	}

	idJointMat *joints = ( idJointMat * )_alloca16( numJoints * sizeof( *joints ) );

	// convert the joint quaternions to joint matrices
	SIMDProcessor->ConvertJointQuatsToJointMats( joints, jointFrame, numJoints );

	// first joint is always root of entire hierarchy
	if ( AFPoseJoints.Num() && AFPoseJoints[0] == 0 ) {
		switch( AFPoseJointMods[0].mod ) {
		case AF_JOINTMOD_AXIS: {
			joints[0].SetRotation( AFPoseJointMods[0].axis );
			break;
							   }
		case AF_JOINTMOD_ORIGIN: {
			joints[0].SetTranslation( AFPoseJointMods[0].origin );
			break;
								 }
		case AF_JOINTMOD_BOTH: {
			joints[0].SetRotation( AFPoseJointMods[0].axis );
			joints[0].SetTranslation( AFPoseJointMods[0].origin );
			break;
							   }
		}
		j = 1;
	} else {
		j = 0;
	}

	// pointer to joint info
	jointParent = modelDef->JointParents();

	// transform the child joints
	for( i = 1; j < AFPoseJoints.Num(); j++, i++ ) {
		jointMod = AFPoseJoints[j];

		// transform any joints preceding the joint modifier
		SIMDProcessor->TransformJoints( joints, jointParent, i, jointMod - 1 );
		i = jointMod;

		parentNum = jointParent[i];

		switch( AFPoseJointMods[jointMod].mod ) {
		case AF_JOINTMOD_AXIS: {
			joints[i].SetRotation( AFPoseJointMods[jointMod].axis );
			joints[i].SetTranslation( joints[parentNum].ToVec3() + joints[i].ToVec3() * joints[parentNum].ToMat3() );
			break;
							   }
		case AF_JOINTMOD_ORIGIN: {
			joints[i].SetRotation( joints[i].ToMat3() * joints[parentNum].ToMat3() );
			joints[i].SetTranslation( AFPoseJointMods[jointMod].origin );
			break;
								 }
		case AF_JOINTMOD_BOTH: {
			joints[i].SetRotation( AFPoseJointMods[jointMod].axis );
			joints[i].SetTranslation( AFPoseJointMods[jointMod].origin );
			break;
							   }
		}
	}

	// transform the rest of the hierarchy
	SIMDProcessor->TransformJoints( joints, jointParent, i, numJoints - 1 );

	// untransform hierarchy
	SIMDProcessor->UntransformJoints( joints, jointParent, 1, numJoints - 1 );

	// convert joint matrices back to joint quaternions
	SIMDProcessor->ConvertJointMatsToJointQuats( AFPoseJointFrame.Ptr(), joints, numJoints );

	// find all modified joints and their parents
	bool *blendJoints = (bool *) _alloca16( numJoints * sizeof( bool ) );
	memset( blendJoints, 0, numJoints * sizeof( bool ) );

	// mark all modified joints and their parents
	for( i = 0; i < AFPoseJoints.Num(); i++ ) {
		for( jointNum = AFPoseJoints[i]; jointNum != INVALID_JOINT; jointNum = jointParent[jointNum] ) {
			blendJoints[jointNum] = true;
		}
	}

	// lock all parents of modified joints
	AFPoseJoints.SetNum( 0, false );
	for ( i = 0; i < numJoints; i++ ) {
		if ( blendJoints[i] ) {
			AFPoseJoints.Append( i );
		}
	}

	AFPoseBounds = bounds;
	AFPoseTime = time;

	ForceUpdate();
}

void idAnimator::SetAFPoseBlendWeight( float blendWeight )
{
	AFPoseBlendWeight = blendWeight;
}

bool idAnimator::BlendAFPose( idJointQuat *blendFrame ) const
{
	if ( !AFPoseJoints.Num() ) {
		return false;
	}

	SIMDProcessor->BlendJoints( blendFrame, AFPoseJointFrame.Ptr(), AFPoseBlendWeight, AFPoseJoints.Ptr(), AFPoseJoints.Num() );

	return true;
}

void idAnimator::ClearAFPose( void )
{
	if ( AFPoseJoints.Num() ) {
		ForceUpdate();
	}
	AFPoseBlendWeight = 1.0f;
	AFPoseJoints.SetNum( 0, false );
	AFPoseBounds.Clear();
	AFPoseTime = 0;
}

void idAnimator::ClearAllAnims( int currentTime, int cleartime )
{
	int	i;

	for( i = 0; i < ANIM_NumAnimChannels; i++ ) {
		Clear( i, currentTime, cleartime );
	}

	ClearAFPose();
	ForceUpdate();
}

jointHandle_t idAnimator::GetJointHandle( const char *name ) const
{
	if ( !modelDef || !modelDef->ModelHandle() ) {
		return INVALID_JOINT;
	}

	return modelDef->ModelHandle()->GetJointHandle( name );
}

const char * idAnimator::GetJointName( jointHandle_t handle ) const
{
	if ( !modelDef || !modelDef->ModelHandle() ) {
		return "";
	}

	return modelDef->ModelHandle()->GetJointName( handle );
}

int idAnimator::GetChannelForJoint( jointHandle_t joint ) const
{
	if ( !modelDef ) {
		//gameLocal.Error( "idAnimator::GetChannelForJoint: NULL model" );
	}

	if ( ( joint < 0 ) || ( joint >= numJoints ) ) {
		//gameLocal.Error( "idAnimator::GetChannelForJoint: invalid joint num (%d)", joint );
	}

	return modelDef->GetJoint( joint )->channel;
}

bool idAnimator::GetJointTransform( jointHandle_t jointHandle, int currentTime, noVec3 &offset, noMat3 &axis )
{
	if ( !modelDef || ( jointHandle < 0 ) || ( jointHandle >= modelDef->NumJoints() ) ) {
		return false;
	}

	CreateFrame( currentTime, false );

	offset = joints[ jointHandle ].ToVec3();
	axis = joints[ jointHandle ].ToMat3();

	return true;
}

bool idAnimator::GetJointLocalTransform( jointHandle_t jointHandle, int currentTime, noVec3 &offset, noMat3 &axis )
{
	if ( !modelDef ) {
		return false;
	}

	const idList<jointInfo_t> &modelJoints = modelDef->Joints();

	if ( ( jointHandle < 0 ) || ( jointHandle >= modelJoints.Num() ) ) {
		return false;
	}

	// FIXME: overkill
	CreateFrame( currentTime, false );

	if ( jointHandle > 0 ) {
		idJointMat m = joints[ jointHandle ];
		m /= joints[ modelJoints[ jointHandle ].parentNum ];
		offset = m.ToVec3();
		axis = m.ToMat3();
	} else {
		offset = joints[ jointHandle ].ToVec3();
		axis = joints[ jointHandle ].ToMat3();
	}

	return true;
}

const animFlags_t idAnimator::GetAnimFlags( int animNum ) const
{
	animFlags_t result;

	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->GetAnimFlags();
	}

	memset( &result, 0, sizeof( result ) );
	return result;
}

int idAnimator::NumFrames( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->NumFrames();
	} else {
		return 0;
	}
}

int idAnimator::NumSyncedAnims( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->NumAnims();
	} else {
		return 0;
	}
}

const char					* idAnimator::AnimName( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->Name();
	} else {
		return "";
	}
}

const char					* idAnimator::AnimFullName( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->FullName();
	} else {
		return "";
	}
}

int idAnimator::AnimLength( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->Length();
	} else {
		return 0;
	}
}

const noVec3				& idAnimator::TotalMovementDelta( int animNum ) const
{
	const idAnim *anim = GetAnim( animNum );
	if ( anim ) {
		return anim->TotalMovementDelta();
	} else {
		return vec3_origin;
	}
}

void idAnimator::FreeData( void )
{
	int	i, j;

	/*if ( entity ) {
		entity->BecomeInactive( TH_ANIMATE );
	}*/

	for( i = ANIMCHANNEL_ALL; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++ ) {
			channels[ i ][ j ].Reset( NULL );
		}
	}

	jointMods.DeleteContents( true );

	Mem_Free16( joints );
	joints = NULL;
	numJoints = 0;

	modelDef = NULL;

	ForceUpdate();
}

void idAnimator::PushAnims( int channelNum, int currentTime, int blendTime )
{
	int			i;
	idAnimBlend *channel;

	channel = channels[ channelNum ];
	if ( !channel[ 0 ].GetWeight( currentTime ) || ( channel[ 0 ].starttime == currentTime ) ) {
		return;
	}

	for( i = ANIM_MaxAnimsPerChannel - 1; i > 0; i-- ) {
		channel[ i ] = channel[ i - 1 ];
	}

	channel[ 0 ].Reset( modelDef );
	channel[ 1 ].Clear( currentTime, blendTime );
	ForceUpdate();
}

/*
=====================
idAnimator::SetEntity
=====================
*/
void idAnimator::SetEntity( idEntity *ent ) {
	entity = ent;
}

idRenderModel				* idAnimator::SetModel( const char *modelname )
{
	int i, j;

	FreeData();

	// check if we're just clearing the model
	if ( !modelname || !*modelname ) {
		return NULL;
	}

	modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, modelname, false ) );
	if ( !modelDef ) {
		return NULL;
	}

	idRenderModel *renderModel = modelDef->ModelHandle();
	if ( !renderModel ) {
		modelDef = NULL;
		return NULL;
	}

	// make sure model hasn't been purged
	modelDef->Touch();

	modelDef->SetupJoints( &numJoints, &joints, frameBounds, removeOriginOffset );
	//modelDef->ModelHandle()->Reset();

	// set the modelDef on all channels
	for( i = ANIMCHANNEL_ALL; i < ANIM_NumAnimChannels; i++ ) {
		for( j = 0; j < ANIM_MaxAnimsPerChannel; j++ ) {
			channels[ i ][ j ].Reset( modelDef );
		}
	}

	return modelDef->ModelHandle();
}

idEntity					* idAnimator::GetEntity( void ) const
{
	return entity;
}


/***********************************************************************

	idAnimManager

***********************************************************************/

/*
====================
idAnimManager::idAnimManager
====================
*/
idAnimManager::idAnimManager() {
}

/*
====================
idAnimManager::~idAnimManager
====================
*/
idAnimManager::~idAnimManager() {
	Shutdown();
}

/*
====================
idAnimManager::Shutdown
====================
*/
void idAnimManager::Shutdown( void ) {
	animations.DeleteContents();
	jointnames.Clear();
	jointnamesHash.Free();
}

/*
====================
idAnimManager::GetAnim
====================
*/
idMD5Anim *idAnimManager::GetAnim( const char *name ) {
	idMD5Anim **animptrptr;
	idMD5Anim *anim;

	// see if it has been asked for before
	animptrptr = NULL;
	if ( animations.Get( name, &animptrptr ) ) {
		anim = *animptrptr;
	} else {
		idStr extension;
		idStr filename = name;
		filename.DefaultPath("Doom3/");
		filename.ExtractFileExtension( extension );
		if ( extension != MD5_ANIM_EXT ) {
			return NULL;
		}

		anim = new idMD5Anim();
		if ( !anim->LoadAnim( filename ) ) {
			//// gameLocal.Warning( "Couldn't load anim: '%s'", filename.c_str() );
			delete anim;
			anim = NULL;
		}
		animations.Set( filename, anim );
	}

	return anim;
}

/*
================
idAnimManager::ReloadAnims
================
*/
void idAnimManager::ReloadAnims( void ) {
	int			i;
	idMD5Anim	**animptr;

	for( i = 0; i < animations.Num(); i++ ) {
		animptr = animations.GetIndex( i );
		if ( animptr && *animptr ) {
			( *animptr )->Reload();
		}
	}
}

/*
================
idAnimManager::JointIndex
================
*/
int	idAnimManager::JointIndex( const char *name ) {
	int i, hash;

	hash = jointnamesHash.GenerateKey( name );
	for ( i = jointnamesHash.First( hash ); i != -1; i = jointnamesHash.Next( i ) ) {
		if ( jointnames[i].Cmp( name ) == 0 ) {
			return i;
		}
	}

	i = jointnames.Append( name );
	jointnamesHash.Add( hash, i );
	return i;
}

/*
================
idAnimManager::JointName
================
*/
const char *idAnimManager::JointName( int index ) const {
	return jointnames[ index ];
}

/*
================
idAnimManager::ListAnims
================
*/
void idAnimManager::ListAnims( void ) const {
	int			i;
	idMD5Anim	**animptr;
	idMD5Anim	*anim;
	size_t		size;
	size_t		s;
	size_t		namesize;
	int			num;

	num = 0;
	size = 0;
	for( i = 0; i < animations.Num(); i++ ) {
		animptr = animations.GetIndex( i );
		if ( animptr && *animptr ) {
			anim = *animptr;
			s = anim->Size();
			//// gameLocal.Printf( "%8d bytes : %2d refs : %s\n", s, anim->NumRefs(), anim->Name() );
			size += s;
			num++;
		}
	}

	namesize = jointnames.Size() + jointnamesHash.Size();
	for( i = 0; i < jointnames.Num(); i++ ) {
		namesize += jointnames[ i ].Size();
	}

	//// gameLocal.Printf( "\n%d memory used in %d anims\n", size, num );
	//// gameLocal.Printf( "%d memory used in %d joint names\n", namesize, jointnames.Num() );
}

/*
================
idAnimManager::FlushUnusedAnims
================
*/
void idAnimManager::FlushUnusedAnims( void ) {
	int						i;
	idMD5Anim				**animptr;
	idList<idMD5Anim *>		removeAnims;
	
	for( i = 0; i < animations.Num(); i++ ) {
		animptr = animations.GetIndex( i );
		if ( animptr && *animptr ) {
			if ( ( *animptr )->NumRefs() <= 0 ) {
				removeAnims.Append( *animptr );
			}
		}
	}

	for( i = 0; i < removeAnims.Num(); i++ ) {
		animations.Remove( removeAnims[ i ]->Name() );
		delete removeAnims[ i ];
	}
}

const idDeclSkin * idDeclModelDef::GetSkin( void ) const {
	return skin;
}

const idDeclSkin * idDeclModelDef::GetDefaultSkin( void ) const {
	return skin;
}
