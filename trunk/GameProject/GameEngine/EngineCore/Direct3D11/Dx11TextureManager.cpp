#include <assert.h>
#include <EngineCore/Direct3D11/Direct3D11Renderer.h>
#include "Dx11TextureManager.h"


TextureFactory local_factory;
TextureFactory* tex_factory = &local_factory;

Dx11TextureManager local_tex_manager;
Dx11TextureManager* tex_manager = &local_tex_manager;

bool TextureFactory::CreateResource(const char* filename)
{
	assert(resmanager_);
	Dx11TextureResource* pRes = new Dx11TextureResource;
	if(!pRes)
		return false;

	if(!resmanager_->ReserveMemory(pRes->GetSize()))
		return false;

	if(!pRes->Create(filename, ((Dx11TextureManager*)resmanager_)->renderer_))
		return false;

	if(!resmanager_->InsertResource(pRes->GetId(), pRes))
		return false;

	return true;
}


bool Dx11TextureResource::Create( const char* filename, Renderer* renderer )
{
	filename_ = filename;
	renderer_ = renderer;
	Image img;
	if (img.loadDDS(filename_.c_str()))
		return false;

	resid_ = renderer->addTexture(img);
	resid_ += TEXRES_BASE;

	memsize_ = img.getWidth(0) * img.getHeight(0) * img.getDepth(0);

	return true;	
}

void Dx11TextureResource::Dispose()
{
	resid_ -= TEXRES_BASE;
	renderer_->removeTexture(resid_);
}

bool Dx11TextureResource::Recreate()
{
	// 필요없음
	return true;
}

size_t Dx11TextureResource::GetSize()
{
	return memsize_;
}

bool Dx11TextureResource::IsDisposed()
{
	return ((resid_ < TEXRES_BASE) ? true : false);
}
