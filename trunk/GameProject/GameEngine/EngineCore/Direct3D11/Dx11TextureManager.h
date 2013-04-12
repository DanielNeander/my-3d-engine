#pragma once
#ifndef __TEXTUREMANAGER_H__
#define __TEXTUREMANAGER_H__

#include <EngineCore/Resource/ResourceManager.h>
#include <string>

class Renderer;

static const ResourceId TEXRES_BASE = 200000;

class Dx11TextureResource : public BaseResource
{
public:
	void Clear()
	{		
		memsize_ = 0;
		resid_ = -1;
	}

	bool Create(const char* filename, Renderer* );

	void Dispose();

	bool Recreate();

	size_t GetSize();

	bool IsDisposed();

	ResourceId GetId() { return resid_; }


private:
	std::string filename_;
	TextureID resid_;
	Renderer* renderer_;
	int memsize_;
};


class Dx11TextureManager : public ResManager
{
public:
	Renderer* renderer_;
};

extern Dx11TextureManager* tex_manager;

class TextureFactory
{
public:
	void SetResManager(ResManager* pRM) { resmanager_ = pRM; }
	bool CreateResource(const char* filename);

private:
	ResManager* resmanager_;
};

extern TextureFactory* tex_factory;

#endif