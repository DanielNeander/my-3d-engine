#include "stdafx.h"
#pragma hdrstop
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "MD5Model.h"
#include "MD5ModelManager.h"
#include "MD5Triangle.h"


class idRenderModelManagerLocal : public idRenderModelManager {
public:
	idRenderModelManagerLocal();
	virtual					~idRenderModelManagerLocal() {}

	virtual void			Init();
	virtual void			Shutdown();
	virtual idRenderModel *	AllocModel();
	virtual void			FreeModel( idRenderModel *model );
	virtual idRenderModel *	FindModel( const char *modelName );
	virtual idRenderModel *	CheckModel( const char *modelName );
	virtual idRenderModel *	DefaultModel();
	virtual void			AddModel( idRenderModel *model );
	virtual void			RemoveModel( idRenderModel *model );
	virtual void			ReloadModels( bool forceAll = false );
	virtual void			FreeModelVertexCaches();
	virtual void			WritePrecacheCommands( idFile *file );
	virtual void			BeginLevelLoad();
	virtual void			EndLevelLoad();

	//virtual	void		PrintMemInfo( MemInfo_t *mi );

private:
	idList<idRenderModel*>	models;
	idHashIndex				hash;
	idRenderModel *			defaultModel;
	idRenderModel *			beamModel;
	idRenderModel *			spriteModel;
	idRenderModel *			trailModel;
	bool					insideLevelLoad;		// don't actually load now

	idRenderModel *			GetModel( const char *modelName, bool createIfNotFound );

	static void				PrintModel_f( const idCmdArgs &args );
	static void				ListModels_f( const idCmdArgs &args );
	static void				ReloadModels_f( const idCmdArgs &args );
	static void				TouchModel_f( const idCmdArgs &args );
};

idRenderModelManagerLocal::idRenderModelManagerLocal()
{
	defaultModel = NULL;
	beamModel = NULL;
	spriteModel = NULL;
	insideLevelLoad = false;
	trailModel = NULL;
}

void idRenderModelManagerLocal::Init()
{

}

void idRenderModelManagerLocal::Shutdown()
{
	models.DeleteContents( true );
	hash.Free();
}

idRenderModel * idRenderModelManagerLocal::AllocModel()
{
	return new idRenderModelStatic();
}

void idRenderModelManagerLocal::FreeModel( idRenderModel *model )
{
	if ( !model ) {
		return;
	}

	/*if ( !dynamic_cast<idRenderModelStatic *>( model ) ) {
	common->Error( "idRenderModelManager::FreeModel: model '%s' is not a static model", model->Name() );
	return;
	}
	if ( model == defaultModel ) {
	common->Error( "idRenderModelManager::FreeModel: can't free the default model" );
	return;
	}
	if ( model == beamModel ) {
	common->Error( "idRenderModelManager::FreeModel: can't free the beam model" );
	return;
	}
	if ( model == spriteModel ) { 
	common->Error( "idRenderModelManager::FreeModel: can't free the sprite model" );
	return;
	}

	R_CheckForEntityDefsUsingModel( model );*/

	delete model;
}

idRenderModel * idRenderModelManagerLocal::FindModel( const char *modelName )
{
	return GetModel( modelName, true );
}

idRenderModel * idRenderModelManagerLocal::CheckModel( const char *modelName )
{
	return GetModel( modelName, false );
}

idRenderModel * idRenderModelManagerLocal::DefaultModel()
{
	return defaultModel;

}

void idRenderModelManagerLocal::AddModel( idRenderModel *model )
{
	hash.Add( hash.GenerateKey( model->Name(), false ), models.Append( model ) );

}

void idRenderModelManagerLocal::RemoveModel( idRenderModel *model )
{
	int index = models.FindIndex( model );
	hash.RemoveIndex( hash.GenerateKey( model->Name(), false ), index );
	models.RemoveIndex( index );
}

void idRenderModelManagerLocal::ReloadModels( bool forceAll /*= false */ )
{

}

void idRenderModelManagerLocal::FreeModelVertexCaches()
{

}

void idRenderModelManagerLocal::WritePrecacheCommands( idFile *file )
{

}

void idRenderModelManagerLocal::BeginLevelLoad()
{

}

void idRenderModelManagerLocal::EndLevelLoad()
{

}

//void idRenderModelManagerLocal::PrintMemInfo( MemInfo_t *mi )
//{
//
//}

idRenderModel * idRenderModelManagerLocal::GetModel( const char *modelName, bool createIfNotFound )
{
	idStr		canonical;
	idStr		extension;

	if ( !modelName || !modelName[0] ) {
		return NULL;
	}

	canonical = modelName;
	canonical.ToLower();

	// see if it is already present
	int key = hash.GenerateKey( modelName, false );
	for ( int i = hash.First( key ); i != -1; i = hash.Next( i ) ) {
		idRenderModel *model = models[i];

		if ( canonical.Icmp( model->Name() ) == 0 ) {
			if ( !model->IsLoaded() ) {
				// reload it if it was purged
				model->LoadModel();
			}// else if ( insideLevelLoad && !model->IsLevelLoadReferenced() ) {
			//	// we are reusing a model already in memory, but
			//	// touch all the materials to make sure they stay
			//	// in memory as well
			//	model->TouchData();
			//}
			//model->SetLevelLoadReferenced( true );
			return model;
		}
	}

	// see if we can load it

	// determine which subclass of idRenderModel to initialize

	idRenderModel	*model;

	canonical.ExtractFileExtension( extension );

	if ( ( extension.Icmp( "ase" ) == 0 ) || ( extension.Icmp( "lwo" ) == 0 ) || ( extension.Icmp( "flt" ) == 0 ) ) {
		model = new idRenderModelStatic;
		model->InitFromFile( modelName );
	} else if ( extension.Icmp( "ma" ) == 0 ) {
		model = new idRenderModelStatic;
		model->InitFromFile( modelName );
	} else if ( extension.Icmp( MD5_MESH_EXT ) == 0 ) {
		model = new idRenderModelMD5;
		model->InitFromFile( modelName );
	}/* else if ( extension.Icmp( "md3" ) == 0 ) {
		model = new idRenderModelMD3;
		model->InitFromFile( modelName );
	} else if ( extension.Icmp( "prt" ) == 0  ) {
		model = new idRenderModelPrt;
		model->InitFromFile( modelName );
	} else if ( extension.Icmp( "liquid" ) == 0  ) {
		model = new idRenderModelLiquid;
		model->InitFromFile( modelName );
	} */else {

		if ( extension.Length() ) {
			//common->Warning( "unknown model type '%s'", canonical.c_str() );
		}

		if ( !createIfNotFound ) {
			return NULL;
		}

		/*idRenderModelStatic	*smodel = new idRenderModelStatic;
		smodel->InitEmpty( modelName );
		smodel->MakeDefaultModel();

		model = smodel;*/
	}

	//model->SetLevelLoadReferenced( true );

	if ( !createIfNotFound  /* && model->IsDefaultModel() */) {
		delete model;
		model = NULL;

		return NULL;
	}

	AddModel( model );

	return model;
}

void idRenderModelManagerLocal::PrintModel_f( const idCmdArgs &args )
{

}

void idRenderModelManagerLocal::ListModels_f( const idCmdArgs &args )
{

}

void idRenderModelManagerLocal::ReloadModels_f( const idCmdArgs &args )
{

}

void idRenderModelManagerLocal::TouchModel_f( const idCmdArgs &args )
{

}



idRenderModelManagerLocal	localModelManager;
idRenderModelManager *		renderModelManager = &localModelManager;