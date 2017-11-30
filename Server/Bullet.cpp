#include "Bullet.h"
#include <iostream>
using namespace Objects;
bool Bullet::HitTest(glm::vec2 _pos, float radius)
{
	if (glm::distance(pos, _pos) < radius + rad)
	{
		return true;
	}
	return false;
}

int Objects::Bullet::luaGetPos(lua_State * pState)
{
	Bullet* agent = (Bullet*)GetPointerVar(pState, "self");
	glm::vec2 pos = agent->pos;
	agent->PushFloat(pos.x);
	agent->PushFloat(pos.y);
	return 2;
}

int Objects::Bullet::luaGetVel(lua_State * pState)
{
	Bullet* agent = (Bullet*)GetPointerVar(pState, "self");

	glm::vec2 vel = agent->vel;
	agent->PushFloat(vel.x);
	agent->PushFloat(vel.y);
	return 2;
}

int	 Objects::Bullet::LuaApplyForce(lua_State * pState)
{
	Bullet* agent = (Bullet*)GetPointerVar(pState, "self");
	glm::vec2 force;
	force.x = agent->PopFloat();
	force.y = agent->PopFloat();
	agent->applyForce(force);
	return 0;
}


void Objects::Bullet::applyForce(glm::vec2 & force)
{
	pos += force;
}

Bullet::Bullet()
{
}

int Bullet::Update(float deltaTime, std::unordered_map<int, GameObject>& ClientObjects)
{
	PushFunction("Update");
	PushFloat(deltaTime);
	CallFunction(1, 0);

	if (glm::distance(pos, glm::vec2(1280 / 2, 720 / 2)) > 720)
	{
		dead = true;
		return 0;
	}
	else
		for each (auto object in ClientObjects)
		{
			if (!object.second.dead && !dead && object.first != ID)
				if (HitTest((glm::vec2)object.second.pos, object.second.radius))
				{
					ClientObjects[object.first].dead = true;
					dead = true;
					return object.first;
				}
		}
	return -1;
}

void Bullet::Update(float deltaTime, glm::vec2 res)
{
	pos *= glm::vec2(1280, 720);
	PushFunction("Update");
	PushFloat(deltaTime);
	CallFunction(1, 0);
	//pos += vel * m_speed * deltaTime;
	if (glm::distance(pos, glm::vec2(1280, 720) / 2.f) > 1280 + 200)
	{
		dead = true;
	}
	pos /= glm::vec2(1280, 720);
}

Bullet::Bullet(glm::vec2 startpos, glm::vec4 _colour, glm::vec2 dir, float speed, int id, int num)
{
	pos = startpos;
	colour = _colour;
	vel = dir;
	m_speed = speed;
	ID = id;
	bulletNum = num;
	dead = false;

	RegisterLuaFunction("GetPos", luaGetPos);
	RegisterLuaFunction("GetVel", luaGetVel);
	RegisterLuaFunction("ApplyForce", LuaApplyForce);
}

Bullet::Bullet(GameObject & Go, glm::vec2 dir, float speed)
{
	pos = Go.pos;
	colour = Go.colour;
	vel = dir;
	m_speed = speed;
	dead = false;

	RegisterLuaFunction("GetPos", luaGetPos);
	RegisterLuaFunction("GetVel", luaGetVel);
	RegisterLuaFunction("ApplyForce", LuaApplyForce);
}

Bullet::~Bullet()
{
}
