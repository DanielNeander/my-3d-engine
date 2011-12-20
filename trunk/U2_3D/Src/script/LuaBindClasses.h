#pragma once 


extern void RegisterMeshWithLua(lua_State* pLua);
extern void RegisterTriangleWithLua(lua_State* pLua);
extern void RegisterMeshGroupWithLua(lua_State* pLua);
extern void RegisterSpatialWithLua(lua_State* pLua);

extern void RegisterObjectWithLua(lua_State* pLua);
extern void RegisterControllerWithLua(lua_State* pLua);
extern void RegisterTransformInterpControllerWithLua(lua_State* pLua);

extern void RegisterMeshLoaderWithLua(lua_State* pLua);
extern void RegisterNodeWithLua(lua_State* pLua);

extern void RegisterShaderEffectWithLua(lua_State* pLua);

extern void RegisterGetPathFucWithLua(lua_State* pLua);
extern void RegisterShaderFuncsWithLua(lua_State* pLua);

extern void RegisterAllWithLua(lua_State* pLua);

extern void RegisterGeneralWithLua(lua_State* pLua);