#include "Agent.h"
#include <random>
#include "../Client/GameObject.h"
#include <BitStream.h>
#include <iostream>
#include <time.h>
using namespace Objects;
using glm::vec2;
vec2 trunc(vec2& vec, float max)
{
	float i = max / glm::length(vec);
	i = (i < 1) ? i : 1;
	return vec * i;
}
float Agent::GetFireDelay(int gamemode)
{
	return fireDelay;
}
void Agent::GameModeChange(int gamemode)
{
	switch (gamemode)
	{
	case 1:
	{
		fireDelay = .5f;
		maxVelocity = 100;
		rad = 15;
		radius = 50;
		break;
	}
	case 2:
	{
		fireDelay = 0;
		maxVelocity = 1200;
		rad = 10;
		radius = 65;
		break;
	}
	default:
	{
		//default gameMode
		fireDelay = 0;
		maxVelocity = 600;
		rad = 10;
		radius = 50;
		break;
	}
	}
}
void Agent::Update(float deltaTime)
{
	vec2 desired = *mousePos - pos;
	float dist = glm::length(desired);
	if (dist != 0)
	{
		float slowScale = std::fmin((dist / radius), (float)1);
		vec2 v = glm::normalize(desired);
		vec2 force = v * maxVelocity * slowScale;
		if (slowScale != 1 && glm::length(velocity) != 0)
		{
			float d = std::abs(glm::dot(v, velocity));
			vec2 res = glm::normalize(velocity) * -1.f * (float)velocity.length() * d * (maxVelocity / 100);
			force += res;
		}
		velocity = velocity + (force - velocity) * deltaTime;
		pos += velocity * deltaTime;
		heading = glm::normalize(velocity);
	}
}

Bullet* Agent::Fire(glm::vec4 colour, int _id, int gameMode)
{
	switch (gameMode)
	{
	case 1:
	{
		return new Bullet(pos, colour, glm::normalize(pos - *aimPos), 1200, id, _id);
		break;
	}
	case 2:
	{
		return new Bullet(pos, colour, glm::normalize(*aimPos - pos), 300, id, _id);
		break;
	}
	default:
	{
		//default gameMode
		return new Bullet(pos, colour, glm::normalize(*aimPos - pos), 600, id, _id);
		break;
	}
	}

	//return new Bullet(pos, colour, glm::normalize(*aimPos - pos), 600, id, _id);
}


Agent::Agent(glm::vec2* aim, glm::vec2* mouseFollow)
{
	std::default_random_engine generator((unsigned int)time(NULL));
	std::uniform_int_distribution<int> x(10, 1270);
	std::uniform_int_distribution<int> y(10, 710);
	aimPos = aim;
	mousePos = mouseFollow;
	pos = vec2(x(generator), y(generator));
}


Agent::~Agent()
{
}
