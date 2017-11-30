#include "LuaObject.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>

LuaObject::LuaObject()
{
	m_luaState = luaL_newstate();
	luaL_openlibs(m_luaState);

	// Pass the "self" pointer to lua so that it can be used in scripts
	SetPointerVar(m_luaState, "self", this);	
}


LuaObject::~LuaObject()
{
	lua_close(m_luaState);
}

bool LuaObject::LoadScript(const char* szPath)
{
	int status = luaL_dofile(m_luaState, szPath);
	if (status) {
		std::cout << "Couldn't load script: " << szPath << std::endl << lua_tostring(m_luaState, -1);
		return false;
	}


	// run the Init function in the script.
	// Start by putting the function on the stack
	lua_getglobal(m_luaState, "Init");
	// now execut it
	status = lua_pcall(m_luaState, 0, 0, NULL);
	if (status) {
		std::cout << "Couldn't execute function Init " << lua_tostring(m_luaState, -1) << std::endl;
	}

	return true;
}

void LuaObject::RegisterLuaFunction(const char* szFuncName, lua_CFunction fcnFunction)
{
	lua_register(m_luaState, szFuncName, fcnFunction);
}

bool LuaObject::CallFunction(int argCount, int returnCount)
{
	if (!lua_isfunction(m_luaState, -(argCount + 1))) {
		std::cout << "BaseLuaObject::callFunction - function is not in the correct position on the stack";
		return false;
	}

	int status = lua_pcall(m_luaState, argCount, returnCount, NULL);

	if (status) {
		std::cout << "Couldn't execute function " << lua_tostring(m_luaState, -1);
		return false;
	}

	return true;
}

void LuaObject::SetPointerVar(lua_State* pState, const char* pVarName, void* vpVal)
{
	lua_pushlightuserdata(pState, vpVal);
	lua_setglobal(pState, pVarName);
}

void* LuaObject::GetPointerVar(lua_State* pState, const char* pVarName)
{
	lua_getglobal(pState, pVarName);

	if (lua_isuserdata(pState, -1) == false) {
		std::cout << "BaseLuaObject::getPointerVar: Variable is not a pointer";
		return NULL;
	}
	else {
		void* vpVal = (void*)lua_topointer(pState, -1);
		lua_pop(pState, 1);
		return vpVal;
	}
}

void LuaObject::PushFloat(float fValue)
{
	lua_pushnumber(m_luaState, fValue);
}

float LuaObject::PopFloat()
{
	if (lua_isnumber(m_luaState, -1) == false) {
		int iStackSize = lua_gettop(m_luaState);
		std::cout << "BaseLuaObject::popFloat: Variable is not a number";
		return 0.0f;
	}
	else {
		float fVal = (float)lua_tonumber(m_luaState, -1);
		lua_pop(m_luaState, 1);
		return fVal;
	}
}

void LuaObject::PushFunction(const char* szFunction)
{
	lua_getglobal(m_luaState, szFunction);
	if (lua_isfunction(m_luaState, -1) == false) {
		std::cout << "BaseLuaObject::pushFunction: variable is not a function";
		return;
	}
}
std::vector<std::string> splitString(std::string str)
{
	std::istringstream iss(str);
	return std::vector<std::string>(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>());
}
std::vector<std::vector<std::string>> LuaObject::ScriptFromFileToString(std::string path)
{
	std::ifstream scriptFile(path, std::ios::in);
	std::string text;
	std::vector<std::vector<std::string>> list;
	while (scriptFile.good())
	{
		if (std::getline(scriptFile, text))
		{
			list.push_back(splitString(text));
		}
	}
	return list;
}

void LuaObject::SaveScriptFromStringArray(const char* string, int id)
{
	std::string str = "bullet" + std::to_string(id) + ".lua";
	std::ofstream(str.c_str()) << string;
}

void LuaObject::SendScript(std::vector<std::vector<std::string>> script, int id, RakNet::RakPeerInterface* pPeerInterface)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)ID_LUASCRIPT);
	bs.Write(id);
	RakNet::RakString temp;
	for each (auto Line in script)
	{
		for each (auto string in Line)
		{
			temp += string.c_str();
			temp += " ";
		}
		temp += '\n';
	}
	bs.Write(temp);
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}
