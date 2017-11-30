#pragma once
#include <lua.hpp>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "../bootstrap/GameMessages.h"
#include <vector>
#include <memory>
class LuaObject
{
public:
	LuaObject();
	~LuaObject();

	bool LoadScript(const char * szPath);
	void RegisterLuaFunction(const char* szFuncName, lua_CFunction fcnFunction);
	bool CallFunction(int argCount, int returnCount);
	static void	 SetPointerVar(lua_State* pState, const char* pVarName, void* vpVal);
	static void* GetPointerVar(lua_State* pState, const char* pVarName);
	void PushFloat(float fValue);
	float PopFloat();
	void PushFunction(const char* szFunction);
	static std::vector<std::vector<std::string>> ScriptFromFileToString(std::string path);
	static void SaveScriptFromStringArray(const char * string, int id);
	static void SendScript(std::vector<std::vector<std::string>> script, int id, RakNet::RakPeerInterface* pPeerInterface);
	lua_State* m_luaState;
};

