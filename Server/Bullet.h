#pragma once
#include "../dependencies/glm/glm/glm.hpp"
#include "../Client/GameObject.h"
#include <unordered_map>
#include "LuaObject.h"
namespace Objects
{
	class Bullet : public LuaObject, public GameObject
	{
	public:
		float m_speed;
		float rad = 4;
		int bulletNum = 0;
		int Update(float deltaTime, std::unordered_map<int, GameObject>& ClientObjects);
		void Update(float deltaTime, glm::vec2 res);
		bool HitTest(glm::vec2 _pos, float radius);

		static int luaGetPos(lua_State* pState);
		static int luaGetVel(lua_State* pState);
		static int LuaApplyForce(lua_State* pState);
		void applyForce(glm::vec2 & force);

		Bullet();
		Bullet(glm::vec2 startpos, glm::vec4 colour, glm::vec2 dir, float speed, int ID, int num);
		Bullet(GameObject& Go, glm::vec2 dir, float speed);
		~Bullet();
	};
}
